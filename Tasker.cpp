#pragma once
//#include "pch.h"
//#include "stdafx.h"
#include "Tasker.h"



Tasker::Tasker()
{
	_rtype = request_type::EMPTY;
	proc = true;

	_task.rtype = request_type::EMPTY;
	_task.start_time.clear();
	_task.end_time.clear();
	_task.start_db_time.clear();
	_task.end_db_time.clear();
	_task.grz.clear();
	_task.count = XML_COUNT;

	for (size_t i = 0; i < (viola_type::SIZE_VIOLA_TYPE - 1); i++)
		_task.type[i] = viola_type::SPACE;

}


Tasker::~Tasker()
{
}

int Tasker::createTask(char *p, bool begin)
{
	memset(_get ,0, TEMPS_SIZE);
	if (p != nullptr)
	{
		for (int i= 0;i<strlen(p);i++)
			_get[i] = (char)tolower(p[i]);
	}
	//strcpy(_get, p);

	////2018-05-24T15:03:21+03
	if (_rtype == request_type::T1)
	{
		_task.start_time = time_convert(_get);
		_rtype = request_type::EMPTY;
	}

	//2018-05-24T15:03:21+03
	if (_rtype == request_type::T2)
	{
		_task.end_time = time_convert(_get);
		_rtype = request_type::EMPTY;
	}

	//2019-05-24T15:03:21+04:00
	if (_rtype == request_type::TDB1)
	{
		_task.start_db_time = time_convert(_get);
		_rtype = request_type::EMPTY;
	}

	//2019-05-24T15:03:21+04:00
	if (_rtype == request_type::TDB2)
	{
		_task.end_db_time = time_convert(_get);
		_rtype = request_type::EMPTY;
	}

	if (_rtype == request_type::TYPE)
	{
		if (strcmp(_get, "viol") == 0)
		{
			for (size_t i = 0; i < (viola_type::SIZE_VIOLA_TYPE - 1); i++)
				_task.type[i] = (i <= viola_type::C16) ? (viola_type)(i+1) : viola_type::SPACE;
			_rtype = request_type::EMPTY;
		}
		else
		{
			char *_get_pch = nullptr;
			char *next_token1 = nullptr;

			memset(_get_viola, 0, TEMPS_SIZE);
			strcpy(_get_viola, _get);
			int cnt = 0;
			char del[2] = { ',',0 };
			_get_pch = strtok_s(_get_viola, del, &next_token1);

			_get_pch[0] = ' ';
			_task.type[cnt] = (viola_type)atoi(_get_pch);

			while (_get_pch != nullptr)
			{
				cnt++;
			_get_pch = strtok_s(nullptr, del, &next_token1);
				if (_get_pch == nullptr)break;
				_get_pch[0] = ' ';
				_task.type[cnt] = (viola_type)atoi(_get_pch);
			}

			_rtype = request_type::EMPTY;
		}
	}

	if (_rtype == request_type::NUM)
	{
		//todo обезопасить
		_task.grz = std::string(_get);
		_rtype = request_type::EMPTY;
	}

	if (_rtype == request_type::CNT)
	{
		//todo обезопасить
		if(strlen(_get) > 0)
		_task.count = atoi(_get);
		if (_task.count > XML_COUNT) _task.count = XML_COUNT;
		_rtype = request_type::EMPTY;
	}

	//начало запроса
	if (strcmp(_get, "geteventdata") == 0)
	{
		if(!begin)
			if (proc) proc = false;
		_task.rtype = request_type::DATA;
		_rtype = request_type::DATA;
	}
	
	if (strcmp(_get, "geteventlist") == 0)
	{
		if (!begin)
			if (proc) proc = false;
		_task.rtype = request_type::LIST;
		_rtype = request_type::LIST;
	}

	if (strcmp(_get, "t1") == 0)
		_rtype = request_type::T1;

	if (strcmp(_get, "t2") == 0)
		_rtype = request_type::T2;

	if (strcmp(_get, "tdb1") == 0)
		_rtype = request_type::TDB1;

	if (strcmp(_get, "tdb2") == 0)
		_rtype = request_type::TDB2;

	if (strcmp(_get, "type") == 0)
		_rtype = request_type::TYPE;
	
	if (strcmp(_get, "num") == 0)
		_rtype = request_type::NUM;
	
	if (strcmp(_get, "cnt") == 0)
		_rtype = request_type::CNT;

	//конец запроса
	if (strcmp(_get, "") == 0)
		if (proc) proc = false;

	// получаем res - результат парсинга
	// и генерим запрос в Ѕƒ
	if (!proc && !begin)
	{
		addTask(_task);

		proc = true;
		_rtype = request_type::EMPTY;
		
		_task.rtype;
		_task.start_time.clear();
		_task.end_time.clear();
		_task.start_db_time.clear();
		_task.end_db_time.clear();
		_task.grz.clear();
		_task.count = XML_COUNT;

		for (size_t i = 0; i < (viola_type::SIZE_VIOLA_TYPE - 1); i++)
			_task.type[i] = viola_type::SPACE;
	}

	return 0;
}

int Tasker::addTask(task t)
{
	list_mutex.lock();
	task_list.push_back(t);
	list_mutex.unlock();

	return 0;
}
Tasker::task *Tasker::getTask()
{
	get_task.rtype = Tasker::request_type::EMPTY;
	Tasker::task *res = nullptr;

	//todo - обезопасить
	list_mutex.lock();
	if (task_list.size() > 0)
	{
		get_task = task_list.front();
		task_list.erase(task_list.begin());
	}
	list_mutex.unlock();

	if (get_task.rtype == Tasker::request_type::EMPTY)
		return nullptr;


	res = &get_task;

	return res;
}
std::string Tasker::time_convert(char* _buff)
{
	int year, mont, day, hour, min, sec, offset;
	char sign_offset;
	sscanf(_buff, "%04d-%02d-%02dt%02d:%02d:%02d%c%02d", &year, &mont, &day, &hour, &min, &sec, &sign_offset, &offset);
	char temp[24] = { 0 };
	//2018-01-01 00:00:00
	sprintf(temp, "%04d-%02d-%02d %02d:%02d:%02d", year, mont, day, hour, min, sec);
	return std::string(temp);
}

void Tasker::clear_task()
{
	proc = true;
	_rtype = request_type::EMPTY;

	_task.rtype;
	_task.start_time.clear();
	_task.end_time.clear();
	_task.start_db_time.clear();
	_task.end_db_time.clear();
	_task.grz.clear();
	_task.count = XML_COUNT;

	for (size_t i = 0; i < (viola_type::SIZE_VIOLA_TYPE - 1); i++)
		_task.type[i] = viola_type::SPACE;
}