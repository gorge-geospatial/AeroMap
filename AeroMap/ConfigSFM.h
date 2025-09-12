#ifndef CONFIGSFM_H
#define CONFIGSFM_H

#include "Project.h"

class ConfigSFM
{
public:

	ConfigSFM();
	~ConfigSFM();

	XString GetFileName();
	void WriteDefaultFile();

	void SetValue(XString key, int val);

	int GetIntValue(XString key);

private:

	struct EntryType
	{
		XString key;
		XString val;
	};
	std::vector<EntryType> m_entry_list;

private:

	void LoadFile();
	void SaveFile();
};

#endif // #ifndef CONFIGSFM_H
