#include "io/File.hpp"

namespace FW
{

//------------------------------------------------------------------------

File::File(const String& name, Mode mode, bool disableCache)
:   m_name(name),
    m_file(name.getPtr())
{
    const char* modeName;
    QFile::OpenMode openMode;

    switch (mode)
    {
    case Read:
        modeName = "read";
        openMode = QFile::ReadOnly;
        break;
    case Create:
        modeName = "create";
        openMode = QFile::ReadWrite | QFile::Truncate;
        break;
    case Modify:
        modeName = "modify";
        openMode = QFile::ReadWrite;
        break;
    default:
        FW_ASSERT(false);
        return;
    }

    if (!m_file.open(disableCache ? openMode | QFile::Unbuffered : openMode)) {
        setError("Cannot open file '%s' for %s!", m_name.getPtr(), modeName);
    }
}

//------------------------------------------------------------------------

File::~File(void)
{
}

//------------------------------------------------------------------------

File::Mode File::getMode(void) const
{
    QFile::OpenMode mode = m_file.openMode();
    if ((mode & QFile::Truncate) == QFile::Truncate)
        return Create;
    if ((mode & QFile::ReadWrite) == QFile::ReadWrite)
        return Modify;
    return Read;
}

//------------------------------------------------------------------------

bool File::checkWritable(void) const
{
    if (m_file.isWritable())
        return true;
    setError("File '%s' was opened as read-only!", m_name.getPtr());
    return false;
}

//------------------------------------------------------------------------

void File::seek(S64 ofs)
{
    m_file.seek(ofs);
}

//------------------------------------------------------------------------

int File::read(void* ptr, int size)
{
    return std::max<int>(0, m_file.read(reinterpret_cast<char*>(ptr), size));
}

//------------------------------------------------------------------------

void File::write(const void* ptr, int size)
{
    m_file.write(reinterpret_cast<const char*>(ptr), size);
}

//------------------------------------------------------------------------

void File::flush(void)
{
    m_file.flush();
}

}
