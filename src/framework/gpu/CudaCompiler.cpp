/*
 *  Copyright (c) 2009-2011, NVIDIA Corporation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of NVIDIA Corporation nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gpu/CudaCompiler.hpp"
#include "gpu/CudaModule.hpp"
#include "base/DLLImports.hpp"
#include "io/File.hpp"
#include "gui/Window.hpp"

#ifdef _MSC_VER
#   include <process.h>
#endif

#include <stdio.h>

namespace
{
#ifdef _WIN32
#   define NVCC "nvcc.exe"
#   define SEPARATOR "\\"
#   define LIST_SEPARATOR ';'
#else
#   define NVCC "nvcc"
#   define SEPARATOR "/"
#   define LIST_SEPARATOR ':'
#endif
}

using namespace FW;

//------------------------------------------------------------------------

#define SHOW_NVCC_OUTPUT    0

//------------------------------------------------------------------------

String                  CudaCompiler::s_staticCudaBinPath;
String                  CudaCompiler::s_staticOptions;
String                  CudaCompiler::s_staticPreamble;
String                  CudaCompiler::s_staticBinaryFormat;

bool                    CudaCompiler::s_inited          = false;
Hash<U64, Array<U8>*>   CudaCompiler::s_cubinCache;
Hash<U64, CudaModule*>  CudaCompiler::s_moduleCache;
U32                     CudaCompiler::s_nvccVersionHash = 0;
String                  CudaCompiler::s_nvccCommand;

//------------------------------------------------------------------------

CudaCompiler::CudaCompiler(void)
:   m_cachePath             ("cudacache"),
    m_sourceFile            ("unspecified.cu"),
    m_overriddenSMArch      (0),

    m_sourceFileHash        (0),
    m_optionHash            (0),
    m_defineHash            (0),
    m_preambleHash          (0),
    m_memHash               (0),
    m_sourceFileHashValid   (false),
    m_optionHashValid       (false),
    m_defineHashValid       (false),
    m_preambleHashValid     (false),
    m_memHashValid          (false),

    m_window                (NULL)
{
}

//------------------------------------------------------------------------

CudaCompiler::~CudaCompiler(void)
{
}

//------------------------------------------------------------------------

CudaModule* CudaCompiler::compile(bool enablePrints)
{
    staticInit();

    // Cached in memory => done.

    U64 memHash = getMemHash();
    CudaModule** found = s_moduleCache.search(memHash);
    if (found)
        return *found;

    // Compile CUBIN file.

    String cubinFile = compileCubinFile(enablePrints);
    if (!cubinFile.getLength())
        return NULL;

    // Create module and add to memory cache.

    CudaModule* module = new CudaModule(cubinFile);
    s_moduleCache.add(memHash, module);
    return module;
}

//------------------------------------------------------------------------

const Array<U8>* CudaCompiler::compileCubin(bool enablePrints)
{
    staticInit();

    // Cached in memory => done.

    U64 memHash = getMemHash();
    Array<U8>** found = s_cubinCache.search(memHash);
    if (found)
        return *found;

    // Compile CUBIN file.

    String cubinFile = compileCubinFile(enablePrints);
    if (!cubinFile.getLength())
        return NULL;

    // Load CUBIN.

    File in(cubinFile, File::Read);
    S32 size = (S32)in.getSize();
    Array<U8>* cubin = new Array<U8>(NULL, size + 1);
    in.read(cubin->getPtr(), size);
    cubin->set(size, 0);

    // Add to memory cache.

    s_cubinCache.add(memHash, cubin);
    return cubin;
}

//------------------------------------------------------------------------

String CudaCompiler::compileCubinFile(bool enablePrints)
{
    staticInit();

    // Check that the source file exists.
    {
        File file(m_sourceFile, File::Read);
        if (hasError())
            return "";
    }

    // Cache directory does not exist => create it.

    createCacheDir();
    if (hasError())
        return "";

    // Preprocess.

    writeDefineFile();
    String cubinFile, finalOpts;
    runPreprocessor(cubinFile, finalOpts);
    if (hasError())
        return "";

    // CUBIN exists => done.

    if (fileExists(cubinFile))
        return cubinFile;

    // Compile.

    if (enablePrints)
        printf("CudaCompiler: Compiling '%s'...", m_sourceFile.getPtr());
    if (m_window)
        m_window->showModalMessage("Compiling CUDA kernel...\nThis will take a few seconds.");

    runCompiler(cubinFile, finalOpts);

    if (enablePrints)
        printf((hasError()) ? " Failed.\n" : " Done.\n");
    return (hasError()) ? "" : cubinFile;
}

//------------------------------------------------------------------------

void CudaCompiler::staticInit(void)
{
    if (s_inited || hasError())
        return;
    s_inited = true;

    // List potential CUDA and Visual Studio paths.

    Array<String> potentialCudaPaths;

#ifdef _WIN32
    F32 driverVersion = CudaModule::getDriverVersion() / 10.0f;
    Array<String> potentialVSPaths;

    for (char drive = 'C'; drive <= 'E'; drive++)
    {
        for (int progX86 = 0; progX86 <= 1; progX86++)
        {
            String prog = sprintf("%c:\\%s", drive, (progX86 == 0) ? "Program Files" : "Program Files (x86)");
            potentialCudaPaths.add(prog + sprintf("\\NVIDIA GPU Computing Toolkit\\CUDA\\v%.1f", driverVersion));
            potentialVSPaths.add(prog + "\\Microsoft Visual Studio 10.0");
            potentialVSPaths.add(prog + "\\Microsoft Visual Studio 9.0");
            potentialVSPaths.add(prog + "\\Microsoft Visual Studio 8");
        }
        potentialCudaPaths.add(sprintf("%c:\\CUDA", drive));
    }
#else
    potentialCudaPaths.add("/usr");
#endif
    // Query environment variables.

    String pathEnv      = queryEnv("PATH");
    String includeEnv   = queryEnv("INCLUDE");
    String cudaBinEnv   = queryEnv("CUDA_BIN_PATH");
    String cudaIncEnv   = queryEnv("CUDA_INC_PATH");

    // Find CUDA binary path.

    Array<String> cudaBinList;
    if (s_staticCudaBinPath.getLength())
        cudaBinList.add(s_staticCudaBinPath);
    else
    {
        cudaBinList.add(cudaBinEnv);
        splitPathList(cudaBinList, pathEnv);
        for (int i = 0; i < potentialCudaPaths.getSize(); i++)
        {
            cudaBinList.add(potentialCudaPaths[i] + SEPARATOR"bin");
            cudaBinList.add(potentialCudaPaths[i] + SEPARATOR"bin64");
        }
    }

    String cudaBinPath;
    for (int i = 0; i < cudaBinList.getSize(); i++)
    {
        if (!cudaBinList[i].getLength() || !fileExists(cudaBinList[i] + SEPARATOR NVCC))
            continue;

        // Execute "nvcc --version".

#ifdef _MSC_VER
        FILE* pipe = _popen(sprintf("\"%s"SEPARATOR NVCC"\" --version 2>nul", cudaBinList[i].getPtr()).getPtr(), "rt");
#else
        FILE* pipe = popen((String("\"") + cudaBinList[i] + SEPARATOR NVCC"\" --version 2>/dev/null").getPtr(), "r");
#endif
        if (!pipe)
            continue;

        Array<char> output;
        while (!feof(pipe))
            output.add((char)fgetc(pipe));
#ifdef _MSC_VER
        fclose(pipe);
#else
        pclose(pipe);
#endif

        // Invalid response => ignore.

        output.add(0);
        String response(output.getPtr());
        if (!response.startsWith("nvcc: NVIDIA"))
            continue;

        // Hash response.

        cudaBinPath = cudaBinList[i];
        s_nvccVersionHash = hash<String>(response);
        break;
    }

    if (!cudaBinPath.getLength())
        fail("Unable to detect CUDA Toolkit binary path!\nPlease set CUDA_BIN_PATH environment variable.");

    // Find Visual Studio binary path.
#ifdef _MSC_VER
    Array<String> vsBinList;
    splitPathList(vsBinList, pathEnv);
    for (int i = 0; i < potentialVSPaths.getSize(); i++)
        vsBinList.add(potentialVSPaths[i] + "\\VC\\bin");

    String vsBinPath;
    for (int i = 0; i < vsBinList.getSize(); i++)
    {
        if (vsBinList[i].getLength() && fileExists(vsBinList[i] + "\\vcvars32.bat"))
        {
            vsBinPath = vsBinList[i];
            break;
        }
    }

    if (!vsBinPath.getLength())
        fail("Unable to detect Visual Studio binary path!\nPlease run VCVARS32.BAT.");
#endif

    // Find CUDA include path.

    Array<String> cudaIncList;
    cudaIncList.add(cudaBinPath + SEPARATOR".."SEPARATOR"include");
    cudaIncList.add(cudaIncEnv);
    splitPathList(cudaIncList, includeEnv);
#ifdef _WIN32
    cudaIncList.add("C:\\CUDA\\include");
    cudaIncList.add("D:\\CUDA\\include");
#else
    cudaIncList.add("/usr/include");
    cudaIncList.add("/usr/lib/nvidia-cuda-toolkit/include");
#endif

    String cudaIncPath;
    for (int i = 0; i < cudaIncList.getSize(); i++)
    {
        if (cudaIncList[i].getLength() && fileExists(cudaIncList[i] + SEPARATOR"cuda.h"))
        {
            cudaIncPath = cudaIncList[i];
            break;
        }
    }

    if (!cudaIncPath.getLength())
        fail("Unable to detect CUDA Toolkit include path!\nPlease set CUDA_INC_PATH environment variable.");

    // Find Visual Studio include path.

#ifdef _MSC_VER
    Array<String> vsIncList;
    vsIncList.add(vsBinPath + "\\..\\INCLUDE");
    splitPathList(vsIncList, includeEnv);
    for (int i = 0; i < potentialVSPaths.getSize(); i++)
        vsIncList.add(potentialVSPaths[i] + "\\VC\\INCLUDE");

    String vsIncPath;
    for (int i = 0; i < vsIncList.getSize(); i++)
    {
        if (vsIncList[i].getLength() && fileExists(vsIncList[i] + "\\crtdefs.h"))
        {
            vsIncPath = vsIncList[i];
            break;
        }
    }

    if (!vsIncPath.getLength())
        fail("Unable to detect Visual Studio include path!\nPlease run VCVARS32.BAT.");
#endif

    // Form NVCC command line.

#ifdef _MSV_VER
    s_nvccCommand = sprintf("set PATH=%s;%s & "NVCC" -ccbin \"%s\" -I\"%s\" -I\"%s\" -I. -D_CRT_SECURE_NO_DEPRECATE",
        cudaBinPath.getPtr(),
        pathEnv.getPtr(),
        vsBinPath.getPtr(),
        cudaIncPath.getPtr(),
        vsIncPath.getPtr());
#else
    s_nvccCommand = sprintf("PATH=%s:%s "NVCC" -I\"%s\" -I.",
        cudaBinPath.getPtr(),
        pathEnv.getPtr(),
        cudaIncPath.getPtr());
#endif
}

//------------------------------------------------------------------------

void CudaCompiler::staticDeinit(void)
{
    s_staticCudaBinPath = "";
    s_staticOptions = "";
    s_staticPreamble = "";
    s_staticBinaryFormat = "";

    if (!s_inited)
        return;
    s_inited = false;

    flushMemCache();
    s_cubinCache.reset();
    s_moduleCache.reset();
    s_nvccCommand = "";
}

//------------------------------------------------------------------------

void CudaCompiler::flushMemCache(void)
{
    for (int i = s_cubinCache.firstSlot(); i != -1; i = s_cubinCache.nextSlot(i))
        delete s_cubinCache.getSlot(i).value;
    s_cubinCache.clear();

    for (int i = s_moduleCache.firstSlot(); i != -1; i = s_moduleCache.nextSlot(i))
        delete s_moduleCache.getSlot(i).value;
    s_moduleCache.clear();
}

//------------------------------------------------------------------------

String CudaCompiler::queryEnv(const String& name)
{
#ifdef _MSC_VER
    // Query buffer size.

    DWORD bufferSize = GetEnvironmentVariable(name.getPtr(), NULL, 0);
    if (!bufferSize)
        return "";

    // Query value.

    char* buffer = new char[bufferSize];
    buffer[0] = '\0';
    GetEnvironmentVariable(name.getPtr(), buffer, bufferSize);

    // Convert to String.

    String res = buffer;
    delete[] buffer;
    return res;
#else
    const char* tmp = getenv(name.getPtr());
    return tmp ? tmp : "";
#endif
}

//------------------------------------------------------------------------

void CudaCompiler::splitPathList(Array<String>& res, const String& value)
{
    for (int startIdx = 0; startIdx < value.getLength();)
    {
        int endIdx = value.indexOf(LIST_SEPARATOR, startIdx);
        if (endIdx == -1)
            endIdx = value.getLength();

        String item = value.substring(startIdx, endIdx);
        if (item.getLength() >= 2 && item.startsWith("\"") && item.endsWith("\""))
            item = item.substring(1, item.getLength() - 1);
        res.add(item);

        startIdx = endIdx + 1;
    }
}

//------------------------------------------------------------------------

bool CudaCompiler::fileExists(const String& name)
{
    return ((GetFileAttributes(name.getPtr()) & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

//------------------------------------------------------------------------

String CudaCompiler::removeOption(const String& opts, const String& tag, bool hasParam)
{
    String res = opts;
    for (int i = 0; i < res.getLength(); i++)
    {
        bool match = true;
        for (int j = 0; match && j < tag.getLength(); j++)
            match = (i + j < res.getLength() && res[i + j] == tag[j]);
        if (!match)
            continue;

        int idx = res.indexOf(' ', i);
        if (idx != -1 && hasParam)
            idx = res.indexOf(' ', idx + 1);

        res = res.substring(0, i) + ((idx == -1) ? "" : res.substring(idx + 1));
        i--;
    }
    return res;
}

//------------------------------------------------------------------------

U64 CudaCompiler::getMemHash(void)
{
    if (m_memHashValid)
        return m_memHash;

    if (!m_sourceFileHashValid)
    {
        m_sourceFileHash = hash<String>(m_sourceFile);
        m_sourceFileHashValid = true;
    }

    if (!m_optionHashValid)
    {
        m_optionHash = hash<String>(m_options);
        m_optionHashValid = true;
    }

    if (!m_defineHashValid)
    {
        U32 a = FW_HASH_MAGIC, b = FW_HASH_MAGIC, c = FW_HASH_MAGIC;
        for (int i = m_defines.firstSlot(); i != -1; i = m_defines.nextSlot(i))
        {
            a += hash<String>(m_defines.getSlot(i).key);
            b += hash<String>(m_defines.getSlot(i).value);
            FW_JENKINS_MIX(a, b, c);
        }
        m_defineHash = ((U64)b << 32) | c;
        m_defineHashValid = true;
    }

    if (!m_preambleHashValid)
    {
        m_preambleHash = hash<String>(m_preamble);
        m_preambleHashValid = true;
    }

    U32 a = FW_HASH_MAGIC + m_sourceFileHash;
    U32 b = FW_HASH_MAGIC + m_optionHash;
    U32 c = FW_HASH_MAGIC + m_preambleHash;
    FW_JENKINS_MIX(a, b, c);
    a += (U32)(m_defineHash >> 32);
    b += (U32)m_defineHash;
    FW_JENKINS_MIX(a, b, c);
    m_memHash = ((U64)b << 32) | c;
    m_memHashValid = true;
    return m_memHash;
}

//------------------------------------------------------------------------

void CudaCompiler::createCacheDir(void)
{
    DWORD res = GetFileAttributes(m_cachePath.getPtr());
    if (res == 0xFFFFFFFF || (res & FILE_ATTRIBUTE_DIRECTORY) == 0)
        if (CreateDirectory(m_cachePath.getPtr(), NULL) == 0)
            fail("Cannot create CudaCompiler cache directory '%s'!", m_cachePath.getPtr());
}

//------------------------------------------------------------------------

void CudaCompiler::writeDefineFile(void)
{
    File file(m_cachePath + SEPARATOR"defines.inl", File::Create);
    BufferedOutputStream out(file);
    for (int i = m_defines.firstSlot(); i != -1; i = m_defines.nextSlot(i))
        out.writef("#define %s %s\n",
            m_defines.getSlot(i).key.getPtr(),
            m_defines.getSlot(i).value.getPtr());
    out.writef("%s\n", s_staticPreamble.getPtr());
    out.writef("%s\n", m_preamble.getPtr());
    out.flush();
}

//------------------------------------------------------------------------

void CudaCompiler::initLogFile(const String& name, const String& firstLine)
{
    File file(name, File::Create);
    BufferedOutputStream out(file);
    out.writef("%s\n", firstLine.getPtr());
    out.flush();
}

//------------------------------------------------------------------------

void CudaCompiler::runPreprocessor(String& cubinFile, String& finalOpts)
{
    // Preprocess.

    finalOpts = "";
    if (s_staticOptions.getLength())
        finalOpts += s_staticOptions + " ";
    finalOpts += m_options;

    String logFile = m_cachePath + SEPARATOR"preprocess.log";
    String cmd = sprintf("%s -E -o \"%s"SEPARATOR"preprocessed.cu\" -include \"%s"SEPARATOR"defines.inl\" %s \"%s\" 2>>\"%s\"",
        s_nvccCommand.getPtr(),
        m_cachePath.getPtr(),
        m_cachePath.getPtr(),
        finalOpts.getPtr(),
        m_sourceFile.getPtr(),
        logFile.getPtr());

    initLogFile(logFile, cmd);
    if (system(cmd.getPtr()) != 0)
    {
        setLoggedError("CudaCompiler: Preprocessing failed!", logFile);
        return;
    }

    // Specify binary format.

    if (s_staticBinaryFormat.getLength())
        finalOpts += s_staticBinaryFormat;
    else
        finalOpts += "-cubin";
    finalOpts += " ";

    // Hash and find inline compiler options.

    String optionPrefix = "// EMIT_NVCC_OPTIONS ";
    File file(m_cachePath + SEPARATOR"preprocessed.cu", File::Read);
    BufferedInputStream in(file);

    U32 hashA = FW_HASH_MAGIC;
    U32 hashB = FW_HASH_MAGIC;
    U32 hashC = FW_HASH_MAGIC;

    for (int lineIdx = 0;; lineIdx++)
    {
        const char* linePtr = in.readLine();
        if (!linePtr)
            break;

        // Trim from the left.

        while (*linePtr == ' ' || *linePtr == '\t')
            linePtr++;

        // Directive or empty => ignore.

        if (*linePtr == '#' || *linePtr == '\0')
            continue;

        // Compiler option directive => record.

        String line(linePtr);
        if (line.startsWith(optionPrefix))
            finalOpts += line.substring(optionPrefix.getLength()) + " ";

        // Not a comment => hash.

        else if (!line.startsWith("//"))
        {
            hashA += hash<String>(line);
            FW_JENKINS_MIX(hashA, hashB, hashC);
        }
    }

    // Override SM architecture.

    S32 smArch = m_overriddenSMArch;
    if (!smArch && CudaModule::isAvailable())
        smArch = CudaModule::getComputeCapability();

    if (smArch)
    {
    finalOpts = removeOption(finalOpts, "-arch", true);
    finalOpts = removeOption(finalOpts, "--gpu-architecture", true);
    finalOpts += sprintf("-arch sm_%d ", smArch);
    }

    // Override pointer width.
    // CUDA 3.2 => requires -m32 for x86 build and -m64 for x64 build.

    if (CudaModule::getDriverVersion() >= 32)
    {
        finalOpts = removeOption(finalOpts, "-m32", false);
        finalOpts = removeOption(finalOpts, "-m64", false);
        finalOpts = removeOption(finalOpts, "--machine", true);

#if FW_64
        finalOpts += "-m64 ";
#else
        finalOpts += "-m32 ";
#endif
    }

    // Hash final compiler options and version.

    hashA += hash<String>(finalOpts);
    hashB += s_nvccVersionHash;
    FW_JENKINS_MIX(hashA, hashB, hashC);
    cubinFile = sprintf("%s"SEPARATOR"%08x%08x.cubin", m_cachePath.getPtr(), hashB, hashC);
}

//------------------------------------------------------------------------

void CudaCompiler::runCompiler(const String& cubinFile, const String& finalOpts)
{
    String logFile = m_cachePath + SEPARATOR"compile.log";
    String cmd = sprintf("%s -o \"%s\" -include \"%s"SEPARATOR"defines.inl\" %s \"%s\" 2>>\"%s\"",
        s_nvccCommand.getPtr(),
        cubinFile.getPtr(),
        m_cachePath.getPtr(),
        finalOpts.getPtr(),
        m_sourceFile.getPtr(),
        logFile.getPtr());

    initLogFile(logFile, cmd);
    if (system(cmd.getPtr()) != 0 || !fileExists(cubinFile))
        setLoggedError("CudaCompiler: Compilation failed!", logFile);

#if SHOW_NVCC_OUTPUT
    setLoggedError("", logFile);
    printf("%s\n", getError().getPtr());
    clearError();
#endif
}

//------------------------------------------------------------------------

void CudaCompiler::setLoggedError(const String& description, const String& logFile)
{
    String message = description;
    File file(logFile, File::Read);
    BufferedInputStream in(file);
    in.readLine();
    for (;;)
    {
        const char* linePtr = in.readLine();
        if (!linePtr)
            break;
        if (*linePtr)
            message += '\n';
        message += linePtr;
    }
    setError("%s", message.getPtr());
}

//------------------------------------------------------------------------
