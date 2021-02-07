#pragma once
#define TASK_SIZE 500
#define TEMPS_SIZE 256
#define TYPES_CNT 20
#define XML_COUNT 2000

//#include "stdafx.h"
//#include "pch.h"
#include <string>
#include <vector>
#include <mutex>

class Tasker
{
	public:
		Tasker();
		~Tasker();

		enum viola_type { C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, SPACE, SIZE_VIOLA_TYPE };
		//enum request_type { LIST, DATA };
		enum request_type { LIST, DATA , T1 , T2 , TDB1 , TDB2 , TYPE , NUM , CNT ,	EMPTY};

		enum time_addon { PLUS,MINUS };

		struct task
		{
			request_type rtype;
			std::string start_time;
			std::string end_time;
			std::string start_db_time;
			std::string end_db_time;
			viola_type type[TYPES_CNT];
			std::string grz;
			unsigned int count;
		};

		int createTask(char * p, bool begin);
		int addTask(task t);
		task * getTask();

		std::string time_convert(char * _buff);
		void clear_task();

		std::mutex list_mutex;

	private:

		std::vector<task> task_list;
		bool proc = true;
		request_type _rtype;
		task _task;
		Tasker::task get_task;
		char _get[256];
		char _get_viola[TEMPS_SIZE];


};

