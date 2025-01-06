//CTScriptTestCase2.h 脚本解释器测试用例
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

namespace ns_my_script2
{
	struct _test_case
	{
		string source;
		string result;//"error"=编译错误
	};
	class CTestCase : public vector < _test_case >
	{
	private:
		void AddCase(char const* source, char const* result)
		{
			_test_case tmp;
			tmp.source = source;
			tmp.result = result;
			push_back(tmp);
		}
		bool isResultEqual(Variable const& ret, string const& result)const
		{
			switch (ret.type)
			{
			case Variable::NULLVARIABLE:
				return 0 == result.size();
				break;
			case Variable::LONG:
				return ret.GetLong() == atol(result.c_str());
				break;
			case Variable::DOUBLE:
				return ret.GetDouble() == atof(result.c_str());
				break;
			case Variable::STRING:
				return ret.GetString() == result;
				break;
			default:
				thelog << "未知的类型 " << ret.type << ende;
				return false;
				break;
			}
		}
	public:
		CTestCase()
		{
			AddCase("return 5;", "5");
			AddCase("return -5;", "-5");
			AddCase("return 5.6", "5.6");
			AddCase("return 5.", "5");
			AddCase("return .6", ".6");
			AddCase("int i=5;return ++i", "6");
			AddCase("int i=5;return i++", "5");
			AddCase("int i=5;return --i", "4");
			AddCase("int i=5;return i--", "5");
			AddCase("return", "");
			AddCase("return;", "");
			AddCase("x", "5");
			AddCase("y", "10.5");
			AddCase("z", "abc");
			AddCase("return max(x,y)", "10.5");

			AddCase("int i=2.3;i+=1,i+=1;max(i+7,5);", "11");
			AddCase("int i=2.3;i+=1,i+=1;max(i+7,5);int _;b ;return a.b", "error");
			AddCase("int x;return 1;", "error");
			AddCase("int i;if(1)int i;else int i;return 1;", "error");
			AddCase("if(1)int i=2;else int i;return i;", "error");
			AddCase("if(0){int i=2;return i;}else{ int i=3;return i;}", "3");
			AddCase("if(1){int i=2;return i;}else{ int i=3;return i;}", "2");
			AddCase("int i=1;if(i<2)return 3;else return 4", "3");
			AddCase("int i=2;if(i<2)return 3;else return 4", "4");
			AddCase("for(int i=2;i<5;++i){return i;}", "2");
			AddCase("int i=0;for(;i<5;++i){}return i;", "5");
			AddCase("int i=1;for(int i=2;i<5;++i){int i=10;return i;}", "error");
			AddCase("for(int i=2;i<5;++i){int i=10;return i;}", "error");
			AddCase("for(int i=2;i<5;++i){int j=10;return i;}", "2");
			
			AddCase("int a;int f(){return 3;};int b;for(int i=2;i<5;++i){int j=10;return f();}", "3");
			AddCase("int a;int f(int a,int b){return b;};int b;for(int i=2;i<5;++i){int j=10;return f(1,2);}", "error");
			AddCase("int a;int f(int x,int b){return b;};int b;for(int i=2;i<5;++i){int j=10;return f(1,2);}", "error");
			AddCase("int a;int f(int aa,int b){return b;};int f(int aa,int b){return b;};int b=3;for(int i=2;i<5;++i){int j=10;return f(1,2);}", "error");
			AddCase("int a;int f(int aa,int b){return b;};int b=3;for(int i=2;i<5;++i){int j=10;return f(1,2);}", "2");
		}
		bool doTestCase()
		{
			string str;

			CCTScript script;
			ICTScript Script;
			ICTScript* pScript = &Script;
			pScript->AttachScript(&script);

			CMyVector<pair<string, Variable > > envs;
			Variable tmpvar;
			tmpvar = 5L;
			envs.push_back(pair<string, Variable >("x", tmpvar));
			tmpvar.clear();
			tmpvar = 10.5;
			envs.push_back(pair<string, Variable >("y", tmpvar));
			tmpvar.clear();
			tmpvar = "abc";
			envs.push_back(pair<string, Variable >("z", tmpvar));

			long tc_count = 0;
			long tc_count_error = 0;
			for (CTestCase::const_iterator it = begin(); it != end(); ++it)
			{
				++tc_count;
				thelog << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl
					<< "测试用例 " << tc_count << " : " << it->source << endi;
				if (!pScript->Compile(it->source.c_str(), &envs))
				{
					if ("error" == it->result)
					{
						thelog << pScript->GetMessage() << endi;
						thelog << "脚本测试用例成功，编译出错，符合预期" << endi;
						continue;
					}
					else
					{
						thelog << pScript->GetMessage() << ende;
						++tc_count_error;
						break;
						continue;
					}
				}
				//thelog << pScript->Report(str) << endi;

				Variable var;
				if (!pScript->Execute(var, &envs))
				{
					thelog << pScript->GetMessage() << ende;
					thelog << pScript->Report(str) << endi;
					++tc_count_error;
					thelog << "脚本执行失败" << ende;
					break;
					continue;
				}
				if (!isResultEqual(var, it->result))
				{
					thelog << pScript->Report(str) << endi;
					thelog << "脚本测试用例失败，预期结果[" << it->result << "]实际结果[" << var.GetString() << "]" << endi;
					++tc_count_error; break;
					continue;
				}
				else
				{
					thelog << "脚本测试用例成功，预期结果[" << it->result << "]实际结果[" << var.GetString() << "]" << endi;
				}
			}
			if (0 == tc_count_error)
			{
				thelog << "全部用例测试结果正确 " << tc_count << endi;
				return true;
			}
			else
			{
				thelog << "共 " << tc_count << " 个用例，测试失败 " << tc_count_error << " 个" << ende;
				return false;
			}
		}
	};
}
