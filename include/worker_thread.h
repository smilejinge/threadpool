#ifndef __WORKER_THREAD_H__
#define __WORKER_THREAD_H__


#include <stdint.h>

#include "config.h"
#include "first_order_structure.h"
#include "http_helper.h"
#include "thread_base.h"
#include "thread_condition.h"
#include "thread_job.h"
#include "thread_mutex.h"


using Helper::HttpHelper;
using Helper::HttpResponse;


struct UnionRecord;

class CDBProcess;
class CDevicecodeOrderDtc;
class CThreadPoolStatic;
class CTrackingidOrderDtc;
class CUnionRecordDtc;


class CWorkerThread : public CThreadBase
{
public:
	CWorkerThread(Config &config);
	CWorkerThread(Config &config, const std::string &threadname, bool detach = false);
	virtual ~CWorkerThread();

public:
	/* 线程创建前，设置一些线程属性 */
	virtual bool Initialize();
	/* 线程创建后，设置一些运行状态 */
	virtual void Prepare();
	/* 线程运行 */
	virtual void *Run();
	/* 线程退出前，做一些清理操作 */
	virtual void Destroy();
	/* 中断退出线程 */
	virtual bool Interrupt(void **msg);

public:
	void SetJob(CThreadJob *job);
	CThreadJob *GetJob();
	void SetThreadPoolStatic(CThreadPoolStatic *threadpoolstatic);
	CThreadPoolStatic *GetThreadPoolStatic();
	bool HttpDecryptTrackingGet(const std::string &data, HttpResponse &http_response);
	bool GetUnionRecord(const UnionRecord &data, uint32_t &count, UnionRecord **result);
	bool InsertDevicecodeRecord(const std::string &appkey, const DevicecodeOrder &data);
	bool SelectDevicecodeRecord(const std::string &appkey, const DevicecodeOrder &data, uint32_t &count, DevicecodeOrder **result);
	bool InsertTrackingidRecord(const std::string &appkey, const TrackingidOrder &data);
	bool SelectTrackingidRecord(const std::string &appkey, const TrackingidOrder &data, uint32_t &count, TrackingidOrder **result);
	bool InsertAttributeDB(const std::string &appkey, const std::string &table_prefix_key, const std::string &table_suffix, const std::string &sql);
	bool UpdateAttributeDB(const std::string &appkey, const std::string &table_prefix_key, const std::string &table_suffix, const std::string &sql);
	int32_t GetErrNoAttributeDB(const std::string &appkey);
	std::string GetErrMsgAttributeDB(const std::string &appkey);
	std::string EscapeString(const std::string &appkey, const std::string &src);

private:
	bool InitHttp();
	bool InitDtc();
	bool InitUnionRecordDtc(Json::Value &dtcvalue);
	bool InitDevicecodeOrderDtc(Json::Value &dtcvalue);
	bool InitTrackingidOrderDtc(Json::Value &dtcvalue);
	bool InitDB();
	bool InitAttributeDB(Json::Value &dbvalue);

private:
	struct DBConfig;
	typedef std::map<std::string, std::string> STR_STR_MAP;
	typedef std::map<std::string, std::string>::iterator STR_STR_MAP_ITER;
	typedef std::map<std::string, DBConfig *> DB_CONFIG_MAP;
	typedef std::map<std::string, DBConfig *>::iterator DB_CONFIG_MAP_ITER;
	typedef std::map<std::string, CDevicecodeOrderDtc *> DEVICECODE_ORDER_DTC_MAP;
	typedef std::map<std::string, CDevicecodeOrderDtc *>::iterator DEVICECODE_ORDER_DTC_MAP_ITER;
	typedef std::map<std::string, CTrackingidOrderDtc *> TRACKINGID_ORDER_DTC_MAP;
	typedef std::map<std::string, CTrackingidOrderDtc *>::iterator TRACKINGID_ORDER_DTC_MAP_ITER;

	struct DBConfig
	{
		std::string appkey;
		CDBProcess *dbprocess;
		STR_STR_MAP table_map;
		DBConfig()
		{
			appkey = "";
			dbprocess = NULL;
			table_map.clear();
		}
	};
	
private:
	Config &m_config;
	CThreadMutex m_job_mutex;
	CThreadCondition m_job_condition;
	CThreadPoolStatic *m_thread_pool_static;
	CThreadJob *m_thread_job;
	CUnionRecordDtc *m_union_record_dtc;
	HttpHelper m_http_helper;
	uint32_t m_http_timeout;
	std::string m_decrypt_tracking_url;
	DEVICECODE_ORDER_DTC_MAP m_devicecode_order_dtc_map;
	TRACKINGID_ORDER_DTC_MAP m_trackingid_order_dtc_map;
	DB_CONFIG_MAP m_attribute_dbconfig_map;
};


#endif
