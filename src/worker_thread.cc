#include "worker_thread.h"

#include "dbprocess.h"
#include "devicecode_order_dtc.h"
#include "first_order_constant.h"
#include "log.h"
#include "thread_debug.h"
#include "thread_pool_static.h"
#include "trackingid_order_dtc.h"
#include "union_record_dtc.h"


CWorkerThread::CWorkerThread(Config &config)
	: m_config(config)
	, m_job_mutex()
	, m_job_condition(&m_job_mutex)
{
	m_thread_pool_static = NULL;
	m_thread_job = NULL;
	m_union_record_dtc = NULL;
	m_http_timeout = 5;
	m_decrypt_tracking_url = kDefaultDecryptTrackingUrl;
	m_devicecode_order_dtc_map.clear();
	m_trackingid_order_dtc_map.clear();
	m_attribute_dbconfig_map.clear();
}

CWorkerThread::CWorkerThread(Config &config, const std::string &threadname, bool detach /* = false */)
	: CThreadBase(threadname, detach)
	, m_config(config)
	, m_job_mutex()
	, m_job_condition(&m_job_mutex)
{
	m_thread_pool_static = NULL;
	m_thread_job = NULL;
	m_union_record_dtc = NULL;
	m_http_timeout = 5;
	m_decrypt_tracking_url = kDefaultDecryptTrackingUrl;
	m_devicecode_order_dtc_map.clear();
	m_trackingid_order_dtc_map.clear();
	m_attribute_dbconfig_map.clear();
}

CWorkerThread::~CWorkerThread()
{
	m_thread_pool_static = NULL;
	
	if(NULL != m_thread_job)
	{
		delete m_thread_job;
		m_thread_job = NULL;
	}

	if(NULL != m_union_record_dtc)
	{
		delete m_union_record_dtc;
		m_union_record_dtc = NULL;
	}

	for(DEVICECODE_ORDER_DTC_MAP_ITER device_iter = m_devicecode_order_dtc_map.begin(); device_iter != m_devicecode_order_dtc_map.end(); ++ device_iter)
	{
		CDevicecodeOrderDtc *device_dtc = device_iter->second;
		delete device_dtc;
	}
	m_devicecode_order_dtc_map.clear();

	for(TRACKINGID_ORDER_DTC_MAP_ITER track_iter = m_trackingid_order_dtc_map.begin(); track_iter != m_trackingid_order_dtc_map.end(); ++ track_iter)
	{
		CTrackingidOrderDtc *track_dtc = track_iter->second;
		delete track_dtc;
	}
	m_trackingid_order_dtc_map.clear();

	for(DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.begin(); db_iter != m_attribute_dbconfig_map.end(); ++ db_iter)
	{
		DBConfig *db_dtc = db_iter->second;
		delete db_dtc;
	}
	m_attribute_dbconfig_map.clear();
}

bool CWorkerThread::Initialize()
{
	if(false == InitHttp())
	{
		log_error("CWorkerThread Initialize failed, when InitHttp.");
		return false;
	}
	if(false == InitDtc())
	{
		log_error("CWorkerThread Initialize failed, when InitDtc.");
		return false;
	}

	if(false == InitDB())
	{
		log_error("CWorkerThread Initialize failed, when InitDB.");
		return false;
	}
	return true;
}

void CWorkerThread::Prepare()
{
	
}

void *CWorkerThread::Run()
{
	m_job_mutex.Lock();
	
	while(!IsNeedStop() || (NULL != m_thread_job))
	{
		if(NULL == m_thread_job)
		{
			m_job_condition.Wait();
			continue;
		}

		m_thread_job->Run();
		m_thread_job->SetWorkThread(NULL);
		delete m_thread_job;
		m_thread_job = NULL;
		m_thread_pool_static->GiveBackIdleThread(this);
	}

	SetExitMsg("CWorkerThread Stop.");
	m_job_mutex.UnLock();
	
	return static_cast<void *>(const_cast<char *>(GetExitMsg().c_str()));
}

void CWorkerThread::Destroy()
{
	
}

bool CWorkerThread::Interrupt(void **msg)
{
	if(NULL == this || m_stop || !GetThreadID())
	{
		return true;
	}
	SetStopFlag(1);
	m_job_condition.Signal();
	bool ret = Join(msg);
	do
	{
		if(false == ret)
		{
			log_error("join failed, when interrupt.");
			break;
		}

		log_error("thread [%s] normal stopped.", GetThreadName().c_str());
	} while(0);

	return ret;
}

void CWorkerThread::SetJob(CThreadJob *job)
{
	m_job_mutex.Lock();
	job->SetWorkThread(this);
	m_thread_job = job;
	m_job_condition.Signal();
	m_job_mutex.UnLock();
}

CThreadJob *CWorkerThread::GetJob()
{
	return m_thread_job;
}

void CWorkerThread::SetThreadPoolStatic(CThreadPoolStatic *threadpoolstatic)
{
	m_job_mutex.Lock();
	m_thread_pool_static = threadpoolstatic;
	m_job_mutex.UnLock();
}

CThreadPoolStatic *CWorkerThread::GetThreadPoolStatic()
{
	return m_thread_pool_static;
}

bool CWorkerThread::HttpDecryptTrackingGet(const std::string &data, HttpResponse &http_response)
{
	std::string request_url = m_decrypt_tracking_url + data;
	return m_http_helper.Get(request_url, http_response);
}

bool CWorkerThread::GetUnionRecord(const UnionRecord &data, uint32_t &count, UnionRecord **result)
{
	return m_union_record_dtc->Select(data, count, result);
}

bool CWorkerThread::InsertDevicecodeRecord(const std::string &appkey, const DevicecodeOrder &data)
{
	DEVICECODE_ORDER_DTC_MAP_ITER devicecode_iter = m_devicecode_order_dtc_map.find(appkey);
	if(m_devicecode_order_dtc_map.end() == devicecode_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	CDevicecodeOrderDtc *dtc = devicecode_iter->second;
	return dtc->Insert(data);
}

bool CWorkerThread::SelectDevicecodeRecord(const std::string &appkey, const DevicecodeOrder &data, uint32_t &count, DevicecodeOrder **result)
{
	DEVICECODE_ORDER_DTC_MAP_ITER devicecode_iter = m_devicecode_order_dtc_map.find(appkey);
	if(m_devicecode_order_dtc_map.end() == devicecode_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	CDevicecodeOrderDtc *dtc = devicecode_iter->second;
	return dtc->Select(data, count, result);
}

bool CWorkerThread::InsertTrackingidRecord(const std::string &appkey, const TrackingidOrder &data)
{
	TRACKINGID_ORDER_DTC_MAP_ITER tracking_iter = m_trackingid_order_dtc_map.find(appkey);
	if(m_trackingid_order_dtc_map.end() == tracking_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	CTrackingidOrderDtc *dtc = tracking_iter->second;
	return dtc->Insert(data);
}

bool CWorkerThread::SelectTrackingidRecord(const std::string &appkey, const TrackingidOrder &data, uint32_t &count, TrackingidOrder **result)
{
	TRACKINGID_ORDER_DTC_MAP_ITER tracking_iter = m_trackingid_order_dtc_map.find(appkey);
	if(m_trackingid_order_dtc_map.end() == tracking_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	CTrackingidOrderDtc *dtc = tracking_iter->second;
	return dtc->Select(data, count, result);
}

bool CWorkerThread::InsertAttributeDB(const std::string &appkey, const std::string &table_prefix_key, const std::string &table_suffix , const std::string &sql)
{
	DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.find(appkey);
	if(m_attribute_dbconfig_map.end() == db_iter)
	{
		log_error("the appkey [%s] is invalid, sql [%s].", appkey.c_str(), sql.c_str());
		return false;
	}

	DBConfig *db_config = db_iter->second;
	STR_STR_MAP_ITER table_iter = db_config->table_map.find(table_prefix_key);
	if(db_config->table_map.end() == table_iter)
	{
		log_error("the table_prefix_key [%s] is invalid, appkey [%s], sql [%s].", table_prefix_key.c_str(), appkey.c_str(), sql.c_str());
		return false;
	}

	std::string insert_sql = "insert into " + table_iter->second + table_suffix + " " + sql + ";";
	log_debug("InsertAttributeDB exec sql is [%s].", insert_sql.c_str());
	if(0 != db_config->dbprocess->ExecSQL(db_config->dbprocess->GetDBName(), insert_sql.c_str()))
	{
		if(kDuplicateKey == db_config->dbprocess->GetErrNo())
		{
			log_debug("exec sql failed, appkey [%s], sql [%s].", appkey.c_str(), insert_sql.c_str());
		}
		else
		{
			log_error("exec sql failed, appkey [%s], sql [%s].", appkey.c_str(), insert_sql.c_str());
		}
		
		return false;
	}

	log_debug("exec sql success, appkey [%s], sql [%s].", appkey.c_str(), insert_sql.c_str());
	return true;
}

bool CWorkerThread::UpdateAttributeDB(const std::string &appkey, const std::string &table_prefix_key, const std::string &table_suffix, const std::string &sql)
{
	DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.find(appkey);
	if(m_attribute_dbconfig_map.end() == db_iter)
	{
		log_error("the appkey [%s] is invalid, sql [%s].", appkey.c_str(), sql.c_str());
		return false;
	}

	DBConfig *db_config = db_iter->second;
	STR_STR_MAP_ITER table_iter = db_config->table_map.find(table_prefix_key);
	if(db_config->table_map.end() == table_iter)
	{
		log_error("the table_prefix_key [%s] is invalid, appkey [%s], sql [%s].", table_prefix_key.c_str(), appkey.c_str(), sql.c_str());
		return false;
	}

	std::string update_sql = "update " + table_iter->second + table_suffix + " " + sql + ";";
	if(0 != db_config->dbprocess->ExecSQL(db_config->dbprocess->GetDBName(), update_sql.c_str()))
	{
		log_error("exec sql failed, appkey [%s], sql [%s].", appkey.c_str(), update_sql.c_str());
		return false;
	}

	log_debug("exec sql success, appkey [%s], sql [%s].", appkey.c_str(), update_sql.c_str());
	return true;
}

int32_t CWorkerThread::GetErrNoAttributeDB(const std::string &appkey)
{
	DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.find(appkey);
	if(m_attribute_dbconfig_map.end() == db_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	DBConfig *db_config = db_iter->second;
	return db_config->dbprocess->GetErrNo();
}

std::string CWorkerThread::GetErrMsgAttributeDB(const std::string &appkey)
{
	DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.find(appkey);
	if(m_attribute_dbconfig_map.end() == db_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return false;
	}

	DBConfig *db_config = db_iter->second;
	return db_config->dbprocess->GetErrMsg();
}

std::string CWorkerThread::EscapeString(const std::string &appkey, const std::string &src)
{
	std::string dst = src;
	
	DB_CONFIG_MAP_ITER db_iter = m_attribute_dbconfig_map.find(appkey);
	if(m_attribute_dbconfig_map.end() == db_iter)
	{
		log_error("the appkey [%s] is invalid.", appkey.c_str());
		return dst;
	}

	DBConfig *db_config = db_iter->second;

	char *escape = new char[src.length() * 2 + 1];
	unsigned long escape_len = db_config->dbprocess->EscapeString(escape, src.c_str(), src.length());
	if((unsigned long)-1 != escape_len)
	{
		escape[escape_len] = '\0';
		dst = escape;
	}
	else
	{
		log_error("[%s] escapestring [%s] failed.", appkey.c_str(), src.c_str());
	}
	
	delete[] escape;
	escape = NULL;
	
	return dst;
}

bool CWorkerThread::InitHttp()
{
	if(m_config.isMember("http_timeout") && m_config["http_timeout"].isUInt())
	{
		uint32_t http_timeout = m_config["http_timeout"].asUInt();
		if(http_timeout > 0)
		{
			m_http_timeout = http_timeout;
		}
	}
	m_http_helper.SetAttribute(CURLOPT_TIMEOUT_MS, static_cast<int32_t>(m_http_timeout));

	if(m_config.isMember("decrypt_tracking_url") && m_config["decrypt_tracking_url"].isString())
	{
		m_decrypt_tracking_url = m_config["decrypt_tracking_url"].asString();
	}

	return true;
}

bool CWorkerThread::InitDtc()
{
	if(m_config.isMember("union_record_dtc") && m_config["union_record_dtc"].isObject())
	{
		Json::Value &union_record = m_config["union_record_dtc"];
		if(false == InitUnionRecordDtc(union_record))
		{
			log_error("InitUnionRecordDtc failed, config is [%s].", union_record.toStyledString().c_str());
			return false;
		}
	}
	else
	{
		log_error("union_record_dtc is invalid, config is [%s].", m_config.toStyledString().c_str());
		return false;
	}
	
	if(m_config.isMember("devicecode_order_dtc") && m_config["devicecode_order_dtc"].isArray())
	{
		Json::Value &devicecode_order = m_config["devicecode_order_dtc"];
		if(devicecode_order.size() <= 0)
		{
			log_error("devicecode_order_dtc is empty, config is [%s].", m_config.toStyledString().c_str());
			return false;
		}
		for(uint32_t devicecode_order_index = 0; devicecode_order_index < devicecode_order.size(); ++ devicecode_order_index)
		{
			Json::Value &one_dtc = devicecode_order[devicecode_order_index];
			if(false == InitDevicecodeOrderDtc(one_dtc))
			{
				log_error("InitDevicecodeOrderDtc failed, config is [%s].", one_dtc.toStyledString().c_str());
				return false;
			}
		}
	}
	else
	{
		log_error("devicecode_order_dtc is invalid, config is [%s].", m_config.toStyledString().c_str());
		return false;
	}

	if(m_config.isMember("trackingid_order_dtc") && m_config["trackingid_order_dtc"].isArray())
	{
		Json::Value &trackingid_order = m_config["trackingid_order_dtc"];
		if(trackingid_order.size() <= 0)
		{
			log_error("trackingid_order_dtc is empty, config is [%s].", m_config.toStyledString().c_str());
			return false;
		}
		for(uint32_t trackingid_order_index = 0; trackingid_order_index < trackingid_order.size(); ++ trackingid_order_index)
		{
			Json::Value &one_dtc = trackingid_order[trackingid_order_index];
			if(false == InitTrackingidOrderDtc(one_dtc))
			{
				log_error("InitTrackingidOrderDtc failed, config is [%s].", one_dtc.toStyledString().c_str());
				return false;
			}
		}
		
	}
	else
	{
		log_error("trackingid_order_dtc is invalid, config is [%s].", m_config.toStyledString().c_str());
		return false;
	}

	return true;
}

bool CWorkerThread::InitUnionRecordDtc(Json::Value &dtcvalue)
{
	m_union_record_dtc = new CUnionRecordDtc();
	if(NULL == m_union_record_dtc)
	{
		log_error("no enough memory for new CUnionRecordDtc.");
		return false;
	}
	if(false == m_union_record_dtc->Initialize(dtcvalue))
	{
		log_error("union record dtc initialize failed, config is [%s].", dtcvalue.toStyledString().c_str());
	}

	return true;
}

bool CWorkerThread::InitDevicecodeOrderDtc(Json::Value &dtcvalue)
{
	std::string appkey = "";
	if(dtcvalue.isMember("appkey") && dtcvalue["appkey"].isString())
	{
		appkey = dtcvalue["appkey"].asString();
	}
	else
	{
		log_error("appkey is invalid, config is [%s].", dtcvalue.toStyledString().c_str());
		return false;
	}
	CDevicecodeOrderDtc *devicecode_order_dtc = new CDevicecodeOrderDtc();
	if(NULL == devicecode_order_dtc)
	{
		log_error("no enough memory for new CDevicecodeOrderDtc.");
		return false;
	}
	if(false == devicecode_order_dtc->Initialize(dtcvalue))
	{
		log_error("devicecode order dtc initialize failed, config is [%s].", dtcvalue.toStyledString().c_str());
		return false;
	}

	std::pair<DEVICECODE_ORDER_DTC_MAP_ITER, bool> insert_devicecode_order_dtc_pair = m_devicecode_order_dtc_map.insert(make_pair(appkey, devicecode_order_dtc));
	if(false == insert_devicecode_order_dtc_pair.second)
	{
		log_error("insert devicecode order dtc failed.");
		delete devicecode_order_dtc;
		devicecode_order_dtc = NULL;
		return false;
	}

	return true;
}

bool CWorkerThread::InitTrackingidOrderDtc(Json::Value &dtcvalue)
{
	std::string appkey = "";
	if(dtcvalue.isMember("appkey") && dtcvalue["appkey"].isString())
	{
		appkey = dtcvalue["appkey"].asString();
	}
	else
	{
		log_error("appkey is invalid, config is [%s].", dtcvalue.toStyledString().c_str());
		return false;
	}
	CTrackingidOrderDtc *trackingid_order_dtc = new CTrackingidOrderDtc();
	if(NULL == trackingid_order_dtc)
	{
		log_error("no enough memory for new CTrackingidOrderDtc.");
		return false;
	}
	if(false == trackingid_order_dtc->Initialize(dtcvalue))
	{
		log_error("trackingid order dtc initialize faield, config is [%s].", dtcvalue.toStyledString().c_str());
		return false;
	}

	std::pair<TRACKINGID_ORDER_DTC_MAP_ITER, bool> insert_trackingid_order_dtc_pair = m_trackingid_order_dtc_map.insert(make_pair(appkey, trackingid_order_dtc));
	if(false == insert_trackingid_order_dtc_pair.second)
	{
		log_error("insert trackingid order dtc failed.");
		delete trackingid_order_dtc;
		trackingid_order_dtc = NULL;
		return false;
	}

	return true;
}

bool CWorkerThread::InitDB()
{
	if(m_config.isMember("attribute_dbconfig") && m_config["attribute_dbconfig"].isArray())
	{
		Json::Value &attribute_dbconfig = m_config["attribute_dbconfig"];
		if(attribute_dbconfig.size() <= 0)
		{
			log_error("attribute_dbconfig is empty, config is [%s].", m_config.toStyledString().c_str());
			return false;
		}
		for(uint32_t attribute_dbconfig_index = 0; attribute_dbconfig_index < attribute_dbconfig.size(); ++ attribute_dbconfig_index)
		{
			Json::Value &one_db = attribute_dbconfig[attribute_dbconfig_index];
			if(false == InitAttributeDB(one_db))
			{
				log_error("InitAttributeDB failed, config is [%s].", one_db.toStyledString().c_str());
				return false;
			}
		}
	}
	else
	{
		log_error("attribute_dbconfig is invalid, config is [%s].", m_config.toStyledString().c_str());
		return false;
	}

	return true;
}

bool CWorkerThread::InitAttributeDB(Json::Value &dbvalue)
{
	std::string appkey = "";
	if(dbvalue.isMember("appkey") && dbvalue["appkey"].isString())
	{
		appkey = dbvalue["appkey"].asString();
	}
	else
	{
		log_error("appkey is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	DBConfig *pdbconfig = new DBConfig();
	if(NULL == pdbconfig)
	{
		log_error("no enough memory for new DBConfig.");
		return false;
	}
	std::pair<DB_CONFIG_MAP_ITER, bool> db_config_insert_pair = m_attribute_dbconfig_map.insert(make_pair(appkey, pdbconfig));
	if(false == db_config_insert_pair.second)
	{
		log_error("insert attribute dbconfig failed, appkey [%s].", appkey.c_str());
		delete pdbconfig;
		pdbconfig = NULL;
		return false;
	}
	pdbconfig->appkey = appkey;

	DBHost db_host;
	if(dbvalue.isMember("ipaddress") && dbvalue["ipaddress"].isString())
	{
		STRCPY(db_host.host, dbvalue["ipaddress"].asCString());
	}
	else
	{
		log_error("ipaddress is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}
					
	if(dbvalue.isMember("port") && dbvalue["port"].isIntegral())
	{
		db_host.port = dbvalue["port"].asUInt();
	}
	else
	{
		log_error("port is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("user") && dbvalue["user"].isString())
	{
		STRCPY(db_host.user, dbvalue["user"].asCString());
	}
	else
	{
		log_error("user is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("password") && dbvalue["password"].isString())
	{
		STRCPY(db_host.password, dbvalue["password"].asCString());
	}
	else
	{
		log_error("password is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("timeout") && dbvalue["timeout"].isIntegral())
	{
		db_host.connect_timeout = dbvalue["timeout"].asUInt();
	}
	else
	{
		log_error("timeout is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("character") && dbvalue["character"].isString())
	{
		STRCPY(db_host.character, dbvalue["character"].asCString());
	}
	else
	{
		log_error("character is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("dbname") && dbvalue["dbname"].isString())
	{
		STRCPY(db_host.dbname, dbvalue["dbname"].asCString());
	}
	else
	{
		log_error("dbname is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	if(dbvalue.isMember("tableinfo") && dbvalue["tableinfo"].isObject())
	{
		Json::Value &tableinfo = dbvalue["tableinfo"];
		Json::Value::Members table_member = tableinfo.getMemberNames();
		for(Json::Value::Members::iterator tmit = table_member.begin(); tmit != table_member.end(); ++ tmit)
		{
			std::string key = (*tmit);
			std::string value = "";
			if(tableinfo[key].isString())
			{
				value = tableinfo[key].asString();
			}
			else
			{
				log_error("the [%s] field is invalid.", key.c_str());
				return false;
			}

			std::pair<STR_STR_MAP_ITER, bool> insert_table_pair = pdbconfig->table_map.insert(make_pair(key, value));
			if(false == insert_table_pair.second)
			{
				log_error("insert table failed, table key is [%s], value is [%s].", key.c_str(), value.c_str());
				return false;
			}
		}
	}
	else
	{
		log_error("tableinfo is invalid, config is [%s].", dbvalue.toStyledString().c_str());
		return false;
	}

	CDBProcess *dbprocess = new CDBProcess(&db_host);
	if(NULL == dbprocess)
	{
		log_error("no enough memory for new CDBProcess, dbhost [%s].", db_host.host);
		return false;
	}
	if(0 != dbprocess->Open(db_host.dbname))
	{
		log_error("open database [%s] failed.", db_host.dbname);
		return false;
	}
	pdbconfig->dbprocess = dbprocess;

	return true;
}
