#ifndef __THREAD_DATETIME_H__
#define __THREAD_DATETIME_H__


#include <stdio.h>

#include <ctime>
#include <string>


class DateTime
{
public:
	static std::string CurrentTime()
	{
		return Time2String(time(NULL));
	}

	/**
	 * 返回格式：2017-12-24 18:28:38
	 **/
	static std::string Time2String(time_t tt)
	{
		struct tm *local_tm = localtime(&tt);
		char time_array[sizeof("yyyy-mm-dd hh:mm:ss")] = {'\0'};
		std::strftime(time_array, sizeof(time_array), "%Y-%m-%d %H:%M:%S", local_tm);
		return time_array;
	}

	static time_t String2Time(const std::string &time_str)
	{
		struct tm local_tm;
		sscanf(time_str.c_str(), "%d-%d-%d %d-%d-%d",
			   &(local_tm.tm_year),
			   &(local_tm.tm_mon),
			   &(local_tm.tm_mday),
			   &(local_tm.tm_hour),
			   &(local_tm.tm_min),
			   &(local_tm.tm_sec));

		local_tm.tm_year -= 1900;
		local_tm.tm_mon -= 1;
		local_tm.tm_isdst = 0;

		return mktime(&local_tm);
	}

	/**
	 * 返回格式：20171224
	 **/
	static std::string OneDay(int32_t whichday = 0)
	{
		time_t now = time(NULL);
		return OneDay(now, whichday);
	}

	/**
	 * 返回格式：20171224
	 **/
	static std::string OneDay(time_t tt, int32_t whichday)
	{
		time_t sometime = tt + (whichday * 86400);
		struct tm *local_sometime = localtime(&sometime);
		char time_array[sizeof("yyyymmdd")] = {'\0'};
		std::strftime(time_array, sizeof(time_array), "%Y%m%d", local_sometime);
		return time_array;
	}
};


#endif
