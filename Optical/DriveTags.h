#ifndef _OPTICAL_DRIVETAGS_H_
#define _OPTICAL_DRIVETAGS_H_

#include <Generic/HashT>


class DriveTags
{
private:

    class TagData
    {
	iptr	value;
	bool	changed;

    public:
	TagData(iptr nv)
	{
	    value = nv;
	    changed = true;
	}

	inline TagData& operator = (iptr nv)
	{
	    if (value != nv)
	    {
		value = nv;
		changed = true;
	    }
	    return *this;
	}

	inline TagData& operator = (const TagData& oth)
	{
	    *this = oth.value;
	    return *this;
	}

	inline bool collect(iptr &ret)
	{
	    ret = value;
	    if (changed)
	    {
		changed = false;
		return true;
	    }
	    return false;
	}

    };


private:
    HashT<iptr, TagData> hash;

public:
    TagData& operator[] (iptr key)
    {
	return hash[key];
    }

}

#endif
