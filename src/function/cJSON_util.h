//cJSON_util.h cJSON辅助功能类
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include <cJSON/cJSON.h>

namespace ns_my_std
{
	class CJsonUtil
	{
	private:
		static bool __GetJsonParam(cJSON* json, char const* name, string& value, bool isTry)
		{
			cJSON* cjson_param;

			cjson_param = cJSON_GetObjectItem(json, name);
			if (NULL == cjson_param)
			{
				if (!isTry)thelog << "获取 " << name << " 失败" << ende;
				return false;
			}
			if (cJSON_IsString(cjson_param))
			{
				char* pvalue = cJSON_GetStringValue(cjson_param);
				if (NULL == pvalue)
				{
					if (!isTry)thelog << "获取 " << name << " 失败" << ende;
					return false;
				}
				value = pvalue;
			}
			else if (cJSON_IsNumber(cjson_param))
			{
				double tmp = cJSON_GetNumberValue(cjson_param);
				char buf[256];
				sprintf(buf, "%lf", tmp);
				value = buf;
				if (!isTry)DEBUG_LOG << "获取 " << name << " [" << value << "]" << tmp << endi;
			}
			else
			{
				value = "";
				if (!isTry)thelog << "获取 " << name << " 失败，不是字符串或数值 " << cjson_param->type << ende;
				return false;
			}

			return true;
		}
		static bool __GetJsonParam(cJSON* json, char const* name, long& value, bool isTry)
		{
			cJSON* cjson_param;

			cjson_param = cJSON_GetObjectItem(json, name);
			if (NULL == cjson_param)
			{
				if (!isTry)thelog << "获取 " << name << " 失败" << ende;
				return false;
			}
			if (cJSON_IsString(cjson_param))
			{
				char* pvalue = cJSON_GetStringValue(cjson_param);
				if (NULL == pvalue)
				{
					if (!isTry)thelog << "获取 " << name << " 失败" << ende;
					return false;
				}
				value = atoi(pvalue);
			}
			else if (cJSON_IsNumber(cjson_param))
			{
				value = cJSON_GetNumberValue(cjson_param);
			}
			else
			{
				if (!isTry)thelog << "获取 " << name << " 失败，不是字符串或数值 " << cjson_param->type << ende;
				return false;
			}

			if (!isTry)DEBUG_LOG << "获取 " << name << " [" << value << "]" << endi;
			return true;
		}
		static bool __GetJsonParam(cJSON* json, char const* name, int& value, bool isTry)
		{
			long tmp;
			if (__GetJsonParam(json, name, tmp, isTry))
			{
				value = tmp;
				return true;
			}
			else return false;
		}
		static bool __GetJsonParam(cJSON* json, char const* name, char& value, bool isTry)
		{
			string tmp;
			if (__GetJsonParam(json, name, tmp, isTry))
			{
				value = tmp[0];
				return true;
			}
			else return false;
		}
		static bool __GetJsonParam(cJSON* json, char const* name, uint32_t& value, bool isTry)
		{
			long tmp;
			if (__GetJsonParam(json, name, tmp, isTry))
			{
				value = tmp;
				return true;
			}
			else return false;
		}
		static bool __GetJsonParam(cJSON* json, char const* name, bool& value, bool isTry)
		{
			string tmp;
			if (__GetJsonParam(json, name, tmp, isTry))
			{
				value = (atoll(tmp.c_str()) != 0 || 0 == stricmp(tmp.c_str(), "true"));
				return true;
			}
			else return false;
		}
	public:
		static bool _GetJsonParam(cJSON* json, char const* name, string& value) { return __GetJsonParam(json, name, value, false); }
		static bool _GetJsonParam(cJSON* json, char const* name, int& value) { return __GetJsonParam(json, name, value, false); }
		static bool _GetJsonParam(cJSON* json, char const* name, long& value) { return __GetJsonParam(json, name, value, false); }
		static bool _GetJsonParam(cJSON* json, char const* name, char& value) { return __GetJsonParam(json, name, value, false); }
		static bool _GetJsonParam(cJSON* json, char const* name, uint32_t& value) { return __GetJsonParam(json, name, value, false); }
		static bool _GetJsonParam(cJSON* json, char const* name, bool& value) { return __GetJsonParam(json, name, value, false); }

		static bool _TryGetJsonParam(cJSON* json, char const* name, string& value) { return __GetJsonParam(json, name, value, true); }
		static bool _TryGetJsonParam(cJSON* json, char const* name, int& value) { return __GetJsonParam(json, name, value, true); }
		static bool _TryGetJsonParam(cJSON* json, char const* name, long& value) { return __GetJsonParam(json, name, value, true); }
		static bool _TryGetJsonParam(cJSON* json, char const* name, char& value) { return __GetJsonParam(json, name, value, true); }
		static bool _TryGetJsonParam(cJSON* json, char const* name, uint32_t& value) { return __GetJsonParam(json, name, value, true); }
		static bool _TryGetJsonParam(cJSON* json, char const* name, bool& value) { return __GetJsonParam(json, name, value, true); }
	};
	class IConfigItem
	{
	public:
		string name;//名称
		IConfigItem(char const* _name) :name(_name) {}
		virtual bool LoadConfigItem(cJSON*) = 0;
		virtual bool SaveConfigItem(cJSON*) = 0;
		virtual string& ToString(string& ret) = 0;
	};
	struct ConfigItem_long : public IConfigItem
	{
		long* pValue;//指向存储位置
		ConfigItem_long(char const* _name, long* _pValue) :IConfigItem(_name), pValue(_pValue) {}
		virtual bool LoadConfigItem(cJSON* cjson_data)override
		{
			return CJsonUtil::_GetJsonParam(cjson_data, name.c_str(), *pValue);
		}
		virtual bool SaveConfigItem(cJSON* cjson_data)override
		{
			return cJSON_AddNumberToObject(cjson_data, name.c_str(), *pValue);
		}
		virtual string& ToString(string& ret)override
		{
			char buf[64];
			sprintf(buf, "%ld", *pValue);
			return ret = buf;
		}
	};
	struct ConfigItem_bool : public IConfigItem
	{
		bool* pValue;//指向存储位置
		ConfigItem_bool(char const* _name, bool* _pValue) :IConfigItem(_name), pValue(_pValue) {}
		virtual bool LoadConfigItem(cJSON* cjson_data)override
		{
			return CJsonUtil::_GetJsonParam(cjson_data, name.c_str(), *pValue);
		}
		virtual bool SaveConfigItem(cJSON* cjson_data)override
		{
			return cJSON_AddNumberToObject(cjson_data, name.c_str(), *pValue);
		}
		virtual string& ToString(string& ret)override
		{
			char buf[64];
			sprintf(buf, "%ld", (long)*pValue);
			return ret = buf;
		}
	};
	struct ConfigItem_string : public IConfigItem
	{
		string* pValue;//指向存储位置
		ConfigItem_string(char const* _name, string* _pValue) :IConfigItem(_name), pValue(_pValue) {}
		virtual bool LoadConfigItem(cJSON* cjson_data)override
		{
			return CJsonUtil::_GetJsonParam(cjson_data, name.c_str(), *pValue);
		}
		virtual bool SaveConfigItem(cJSON* cjson_data)override
		{
			return cJSON_AddStringToObject(cjson_data, name.c_str(), pValue->c_str());
		}
		virtual string& ToString(string& ret)override
		{
			return ret = *pValue;
		}
	};
}
