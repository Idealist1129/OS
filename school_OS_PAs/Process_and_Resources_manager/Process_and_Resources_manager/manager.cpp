#pragma once
#include"manager.h"
#include<string>
#include<set>
using namespace std;

PCB::PCB(int id, const string &name, PCB*father, int priority, int status)
:m_Pname(name)
{
    m_PID = id;
    m_priority = priority;
    m_status = status;
    m_father = father;
    m_sonNum = 0;
 }

void
PCB::addSon(PCB*son){
    ++m_sonNum;
    m_sons.push_back(son);
}

void
PCB::setStatus(int status) {
	m_status = status;
}

int
pcbManager::release(int rid, int num, PCB* process) {
	if (process->m_resources.count(rid))
	{
		if (process->m_resources[rid] >= num)
		{
			process->m_resources[rid] -= num;
			if (process->m_resources[rid] == 0)
			{
				process->m_resources.erase(rid);
			}
			return RELEASE_SUCCESS;
		}
		else return RELEASE_TOO_MUCH;
	}
	else return CUR_PROCESS_DONT_CONTAIN_THE_RESOURCE;
}

RCB::RCB(int rid, int totalnum){
    m_curUsableNum = totalnum;
	m_maxNum = totalnum;
    m_RID = rid;
}

pcbManager::pcbManager(){
    m_runningProcess =  init();
    m_processNum = 1;
	m_processNameToPCBptr.insert(pair<string, PCB *>("god", NULL));
	for (int i = 0; i != PRIORITY_NUM; ++i)
	{
		vector<PCB*> temp;
		m_readyQueue.push_back(temp);
	}
}

PCB * 
pcbManager::init(){
    if(PCB * temp = creat("god", 0, "root", RUNNING))
    {
        return temp;
    }
	return NULL;
};

PCB* 
pcbManager::creat(const string& fathername, int priority, const string &name, int status){
    PCB* newprocess = NULL;
    if(fathername == "god")
    {
        newprocess = new PCB(0, name, 0,priority, READY);
    }
    else if(priority >= 0 && priority <= 2)
    {
        PCB * fatherptr = getPCBPtr(fathername);
        if(fatherptr)
        {
            newprocess = new PCB(m_processNum, name, fatherptr,priority, status);
            fatherptr->addSon(newprocess);
			m_readyQueue[priority].push_back(newprocess);
            ++m_processNum;
        }
    }
	else return NULL;

    m_processNameToPCBptr[name] = newprocess;
    return newprocess;
}

PCB*
pcbManager::getPCBPtr(const string & str){
	if (m_processNameToPCBptr.count(str))
		return m_processNameToPCBptr[str];
	else
		return NULL;
}


rcbManager::rcbManager(){
    for(int i = 0; i!= 4; ++i)
    {
        RCB *temp = new RCB(i + 1, i + 1);
        m_resouces.push_back(*temp);
		vector<SresourcesUseInformation> waitingqueue;
		m_waitingQueue.push_back(waitingqueue);
    }
	m_name_to_num.insert(pair<string, int>("R1", 1));
	m_name_to_num.insert(pair<string, int>("R2", 2));
	m_name_to_num.insert(pair<string, int>("R3", 3));
	m_name_to_num.insert(pair<string, int>("R4", 4));

}

int 
rcbManager::request(int rid, int num){
    if(rid < 1 || rid > 4||num < 0)
    return WRONG_INPUT;

    if(num > m_resouces[rid - 1].m_maxNum)
    {
        return REQUEST_TOO_MUCH;
    }
	else if(num <= m_resouces[rid - 1].m_curUsableNum)
	{
		m_resouces[rid - 1].m_curUsableNum -= num;
		return REQUEST_SATIABLE;
	}
	else {
		return REQUEST_BLOCKED;
	}
}

int
rcbManager::release(int rid, int num){
	m_resouces[rid - 1].m_curUsableNum += num;
	return rid;
}

Manager::Manager(const pcbManager &pcbM, const rcbManager &rcbM):m_pmanager(pcbM), m_rmanager(rcbM){}

string
Manager::timeOut(){

	string retStr = "";
	PCB*p = m_pmanager.m_runningProcess;
	retStr += "process " + p->m_Pname + " is ready now. \n";

	m_operation_information = TIME_OUT;
	scheduler();
	PCB * next = m_pmanager.m_runningProcess;
	retStr += "process " + next->m_Pname + " is running now.\n";

	return retStr;

}

void
Manager::scheduler() {

	PCB * p = findHighestPriorityProcess();

	if (m_operation_information == POSSIBLE_RACE_CONTROL)
	{
		PCB* curprocess = m_pmanager.m_runningProcess;
		if (p->m_priority <= curprocess->m_priority)
			return;
		else if (p->m_priority > curprocess->m_priority)
		{
			//�ѵ�ǰ������Ϊready������ӵ���Ӧ���ȼ��Ķ�����ȥ
			curprocess->setStatus(READY);
			m_pmanager.m_readyQueue[curprocess->m_priority].push_back(curprocess);
			m_operation_information = LET_NEXT_PROCESS_RUN;
		}
	}

	if (m_operation_information == LET_NEXT_PROCESS_RUN)
	{
			//��p��ԭ�ڵ�ready ������ɾ��
			vector<PCB*>::iterator i;
			i = find(m_pmanager.m_readyQueue[p->m_priority].begin(), m_pmanager.m_readyQueue[p->m_priority].end(), p);
			if (i != m_pmanager.m_readyQueue[p->m_priority].end())
				m_pmanager.m_readyQueue[p->m_priority].erase(i);

			//��p����Ϊrunning
			p->setStatus(RUNNING);
			m_pmanager.m_runningProcess = p;//����pcb manager���������еĽ���
		return;
	}

	if (m_operation_information == RACE_TO_CONTROL)
	{
		//�ѵ�ǰ������Ϊready������ӵ���Ӧ���ȼ��Ķ�����ȥ
		m_pmanager.m_runningProcess->setStatus(READY);
		PCB *cur = m_pmanager.m_runningProcess;
		m_pmanager.m_readyQueue[cur->m_priority].push_back(cur);

		//��p��ԭ�ڵ�ready ������ɾ��
		vector<PCB*>::iterator i;
		i = find(m_pmanager.m_readyQueue[p->m_priority].begin(), m_pmanager.m_readyQueue[p->m_priority].end(), p);
		if (i != m_pmanager.m_readyQueue[p->m_priority].end())
			m_pmanager.m_readyQueue[p->m_priority].erase(i);

		//��p����Ϊrunning
		p->setStatus(RUNNING);
		m_pmanager.m_runningProcess = p;//����pcb manager���������еĽ���


	}

	if (m_operation_information == TIME_OUT)
	{
		PCB *cur = m_pmanager.m_runningProcess;
		if (m_pmanager.m_readyQueue[cur->m_priority].empty())
			return;

		else
		{
			//�ѵ�ǰ������Ϊready������ӵ���Ӧ���ȼ��Ķ�����ȥ
			cur->setStatus(READY);
			m_pmanager.m_readyQueue[cur->m_priority].push_back(cur);

			//������һ�����ȼ���ߵĽ���ִ��
			vector<PCB*>::iterator i;
			i = find(m_pmanager.m_readyQueue[p->m_priority].begin(), m_pmanager.m_readyQueue[p->m_priority].end(), p);
			if (i != m_pmanager.m_readyQueue[p->m_priority].end())
				m_pmanager.m_readyQueue[p->m_priority].erase(i);

			//��p����Ϊrunning
			p->setStatus(RUNNING);
			m_pmanager.m_runningProcess = p;//����pcb manager���������еĽ���
		}

	}

}

PCB*
Manager::findHighestPriorityProcess() {
	if (!m_pmanager.m_readyQueue[2].empty())
	{
		PCB *p = m_pmanager.m_readyQueue[2][0];
		return p;
	}
	else if (!m_pmanager.m_readyQueue[1].empty())
	{
		PCB *p = m_pmanager.m_readyQueue[1][0];
		return p;
	}
	else return m_pmanager.m_readyQueue[0][0];
}

string
Manager::release(const vector<string> &strs, int &sta) {
	if (strs.size() != 3)
		sta = WRONG_INPUT;
	int status = m_pmanager.release(m_rmanager.m_name_to_num[strs[1]], stoi(strs[2]), m_pmanager.m_runningProcess);
	if (status == RELEASE_SUCCESS)
	{
		int releasedResourceID = m_rmanager.release(m_rmanager.m_name_to_num[strs[1]], stoi(strs[2]));

		if (m_rmanager.m_waitingQueue[releasedResourceID - 1].size() > 0 && m_rmanager.m_resouces[releasedResourceID - 1].m_curUsableNum >=
			m_rmanager.m_waitingQueue[releasedResourceID - 1][0].num)//��һ�����̻����Դ
		{
			m_rmanager.m_resouces[releasedResourceID - 1].m_curUsableNum -= m_rmanager.m_waitingQueue[releasedResourceID - 1][0].num;//��Ӧ��Դ����������

			PCB* nextBlockedProcess = m_rmanager.m_waitingQueue[releasedResourceID - 1][0].process;
			nextBlockedProcess->m_resources[releasedResourceID] = nextBlockedProcess->m_waitingResources[releasedResourceID];//�õ���Դ�Ľ���Ҫ�洢�õ�����Դ��Ϣ
			nextBlockedProcess->m_waitingResources.erase(releasedResourceID);//�õ���Դ�Ľ���Ҫ������Դ���Լ���������Ϣ���Ƴ�
			m_rmanager.m_waitingQueue[releasedResourceID - 1].erase(m_rmanager.m_waitingQueue[releasedResourceID - 1].begin());//�����������Ƴ��õ���Դ�Ľ���

			nextBlockedProcess->setStatus(READY);//���øý���Ϊready
			m_pmanager.m_readyQueue[nextBlockedProcess->m_priority].push_back(nextBlockedProcess);//���ý��̼��뵽��Ӧ���ȼ���ready������

			//�ڴ˴��Ѿ�������ͷ���Դ���ܴ����Ķ��й�����������̵�״̬ת������Ҫ��scheduler�е��Ƚ���ʵ�ֿ��ܵ���ռ
			m_operation_information = POSSIBLE_RACE_CONTROL;
			scheduler();
		}

	}
	else if (status == RELEASE_TOO_MUCH || status == CUR_PROCESS_DONT_CONTAIN_THE_RESOURCE)
		status = WRONG_INPUT;

	string retVal = "";
	retVal += "processs " + m_pmanager.m_runningProcess->m_Pname + " is running.\n";
	return retVal;

}

string
Manager::deletethem(const vector<string> &strs, int &status) {

	//�˲�����ɶ���Դ���ͷţ��Լ�ɾ�����̴����Ķ��й���
	//�п��ܻ��漰�������Դ���ͷţ��������漰����Ӧ�������еĽ���״̬ת�����Լ����ܵĽ��̵���
	PCB* p = m_pmanager.getPCBPtr(strs[1]);
	PCB* currunningProcess = m_pmanager.m_runningProcess;
	string name;
	if (!p)status = WRONG_INPUT;

	else {
		name = p->m_Pname;
		killTree(p);

	}

	//����rcbmanager�е�״̬��Ϣ�������ܻ����Դ�Ľ��̴�����״̬ת��Ϊ����
	vector<vector<SresourcesUseInformation>>& blockedqueues = m_rmanager.m_waitingQueue;
	for (int i = 0; i != blockedqueues.size(); ++i)
	{
		PCB * p;

		int blockedqueuesize = blockedqueues[i].size();
		for (int j = 0; j != blockedqueuesize; ++j)
		{
			p = blockedqueues[i][0].process;
			if (p->m_waitingResources[i + 1] <= m_rmanager.m_resouces[i].m_curUsableNum)//������ڿ��Ի����Դ����������
			{
				p->m_resources.insert(pair<int, int>(i + 1, p->m_waitingResources[i + 1]));
				p->m_waitingResources.erase(i + 1);//���ý��̵�������Ϣ����Դ��Ϣ�޸�
				p->setStatus(READY);//���øý���״̬��Ϣ
				m_pmanager.m_readyQueue[p->m_priority].push_back(p);//�Ѹý��̼����������

				blockedqueues[i].erase(blockedqueues[i].begin());//����Դ�������������Ƴ��ý���
				m_rmanager.m_resouces[i].m_curUsableNum -= p->m_waitingResources[i + 1];//����Դ�Ŀ���������
			}
		}
	}


	//���p�ǵ�ǰ���̣�ֻҪ�ҵ���һ�����ȼ���ߵĽ��̿�ʼ���м���
	if (p == currunningProcess)
	{
		m_pmanager.m_runningProcess = NULL;
		m_operation_information = LET_NEXT_PROCESS_RUN;
	}
	else m_operation_information = POSSIBLE_RACE_CONTROL;//�ͷŵĽ��̿�����ռ��ǰ���̣�Ҳ���ܲ���ռ
	scheduler();
	string retVal = "";
	retVal += "process " + name + " and its subtree is deleted.\n";
	return retVal;
}

string 
Manager::request(const vector<string> &strs, int &sta) {
	if (m_rmanager.m_name_to_num.count(strs[1]) == 0 || strs.size() != 3)
	{
		sta = WRONG_INPUT;
	}
	int rid = m_rmanager.m_name_to_num[strs[1]];
	int num = stoi(strs[2]);
	int status = m_rmanager.request(rid, num);
	if (status == WRONG_INPUT || status == REQUEST_TOO_MUCH)
		sta = WRONG_INPUT;
	else if (status == REQUEST_BLOCKED) {
		PCB *p = m_pmanager.m_runningProcess;
		p->setStatus(BLOCKED);//����ǰ���̵�״̬����Ϊ����
		p->m_waitingResources.insert(pair<int, int>(rid, num));//���õ�ǰ���̵�������Ϣ
		SresourcesUseInformation temp;
		temp.num = num;
		temp.process = p;
		m_rmanager.m_waitingQueue[rid - 1].push_back(temp);//��Ӧ����Դ���������м��뵱ǰ����


		//��ʱ�������һ������ִ��
		m_operation_information = LET_NEXT_PROCESS_RUN;
		scheduler();


		string retVal = "";
		retVal += "process " + m_pmanager.m_runningProcess->m_Pname + " is running.\n";
		retVal += "process " + p->m_Pname + " is blocked.\n";
		return retVal;
	}
	else if (status == REQUEST_SATIABLE)
	{
		PCB* p = m_pmanager.m_runningProcess;
		p->m_resources.insert(pair<int, int>(rid, num));
		string retVal = "";
		retVal += "process " + p->m_Pname + " requested and acquired " + strs[2] + " " + strs[1] + "\n";
		return retVal;

	}	
	return "error";
}

string
Manager::listReady() {
	vector<vector<PCB*>> readyqueue = m_pmanager.m_readyQueue;
	string retStr = "";
	for (int i = readyqueue.size() - 1; i >= 0; --i)
	{
		retStr += to_string(i);
		retStr += ":";
		for (int j = 0; j != readyqueue[i].size(); ++j)
		{
			retStr += readyqueue[i][j]->m_Pname;
			if (j != readyqueue[i].size() - 1)
				retStr += "-";
		}
		retStr += "\n";
	}
	return retStr;

}

string
Manager::listBlock() {
	vector<vector<SresourcesUseInformation>> waitingQueue = m_rmanager.m_waitingQueue;
	string retStr = "";
	for (int i = 0; i != waitingQueue.size(); ++i)
	{
		switch (i) {
		case 0:retStr += "R1";
			break;
		case 1:retStr += "R2";
			break;
		case 2:retStr += "R3";
			break;
		case 3:retStr += "R4";
			break;
		}
		retStr += ":";
		vector<SresourcesUseInformation>::iterator it = waitingQueue[i].begin();
		for (;it != waitingQueue[i].end(); ++it)
		{
			retStr += it->process->m_Pname;
			if (it != waitingQueue[i].end() - 1)
			{
				retStr += "-";
			}
		}
		retStr += "\n";

	}
	return retStr;
}

string
Manager::listAvailableResources() {
	const vector<RCB> &rcbs = m_rmanager.m_resouces;
	string retStr = "";
	for (int i = 0; i != rcbs.size(); ++i)
	{
		switch (i) {
		case 0:retStr += "R1";
			break;
		case 1:retStr += "R2";
			break;
		case 2:retStr += "R3";
			break;
		case 3:retStr += "R4";
			break;
		}
		retStr += ":";
		retStr += to_string(rcbs[i].m_curUsableNum);
		retStr += "\n";
	}
	return retStr;
}

string
Manager::printPCB(const vector<string> &strs) {
	PCB*p = m_pmanager.getPCBPtr(strs[1]);
	string retVal = "";
	retVal += "PID: " + to_string(p->m_PID) + "\n";
	retVal += "Pname: " + p->m_Pname + "\n";
	retVal += "FatherName: " + p->m_father->m_Pname + "\n";
	retVal += "SonNames: ";
	for (int i = 0; i != p->m_sons.size(); ++i)
	{
		retVal += p->m_sons[i]->m_Pname + " ";
	}
	retVal += "\n";
	retVal += "Priority: " + to_string(p->m_priority) + "\n";
	retVal += "Status: ";
	switch (p->m_status)
	{
	case READY: retVal += "READY";
		break;
	case BLOCKED: retVal += "BLOCKED";
		break;
	case RUNNING: retVal += "RUNNING";
		break;
	}
	retVal += "\n";

	return retVal;
}

string
Manager::interface(int operationType, const vector<string> &strs, size_t strNum){
    int status = 0;
	string retVal;

    switch (operationType)
		{
		case INIT: {
            return "init finished";
		}
		case CREATE:{//�´����Ľ�������ȵ�ǰ�������ȼ��ߣ�����ռ��ǰ����
			if (strNum != 3)
				return "args num error!\n";

			if (m_pmanager.m_processNameToPCBptr.count(strs[1]))
				return "the process name already exist!\n";

			if (!m_pmanager.creat(m_pmanager.m_runningProcess->m_Pname, stoi(strs[2]), strs[1])) {
				return "the priority must be 0-2!\n";
			}

			if (m_pmanager.m_runningProcess->m_priority < stoi(strs[2]))
			{
				m_operation_information = RACE_TO_CONTROL;
				scheduler();
			}
			return "process " + m_pmanager.m_runningProcess->m_Pname + " is running.\n";
		}
		case DELETE: {
			retVal = deletethem(strs, status);
			if (status == WRONG_INPUT)
				goto error;
			
			return retVal;
		}
		case REQUEST: {
			retVal = request(strs, status);
			if (status == WRONG_INPUT)
				goto error;
			return retVal;
		}
		case RELEASE: {
			retVal = release(strs, status);
			if (status == WRONG_INPUT)
				goto error;
			return retVal;
		}
		case TIME_OUT: {
			return timeOut();
		}
		case LIST_READY: {
			retVal = listReady();
			return retVal;
		}
		case LIST_BLOCKED: {
			retVal = listBlock();
			return retVal;
		}
		case LIST_AVAILABLE_RESOURCES: {
			retVal = listAvailableResources();
			return retVal;
		}
		case PRINT_PCB: {
			retVal = printPCB(strs);
			return retVal;
		}
		case CUR_PROCESS:{
			PCB * p = m_pmanager.m_runningProcess;

			string retVal ="Name: " + p->m_Pname + "\n";
			return retVal;
		}
		case WRONG_INPUT: {
            error:
            return "Wrong input! please check again!";
		}
		default:break;
		}    
		return "ERROR";
}

void
Manager::killTree(PCB * p) {
	map<int, int> &resources = p->m_resources;
	map<int, int> &waitingresources = p->m_waitingResources;

	vector<PCB*> &sons = p->m_sons;
	p->m_father->m_sonNum--;
	vector<PCB*>::iterator it;
	it = find(p->m_father->m_sons.begin(), p->m_father->m_sons.end(), p);
	if (it != p->m_father->m_sons.end())
		p->m_father->m_sons.erase(it);

	if (sons.empty())
	{
		m_pmanager.m_processNameToPCBptr.erase(p->m_Pname);


		//�ͷ����е���Դ, ֻ��Ҫ����Rmanager��״̬�����ù�PCB����Ϊ��������Ҫ��ɾ��
		if (!resources.empty())
		{
			map<int, int>::iterator it = resources.begin();
			for (; it != resources.end(); it++)
			{
				m_rmanager.m_resouces[it->first - 1].m_curUsableNum += it->second;
			}
		}

		//���p�ھ��������У��ʹ����˳�
		if (p->m_status == READY)
		{
			vector<PCB*>::iterator i;

			i = find(m_pmanager.m_readyQueue[p->m_priority].begin(), m_pmanager.m_readyQueue[p->m_priority].end(), p);
			if (i != m_pmanager.m_readyQueue[p->m_priority].end())
				m_pmanager.m_readyQueue[p->m_priority].erase(i);
		}

		//���P�����������У��˳�
		if (!waitingresources.empty())
		{
			map<int, int>::iterator it = waitingresources.begin();
			for (; it != waitingresources.end(); it++)
			{
				vector<SresourcesUseInformation>::iterator i;
				SresourcesUseInformation com;
				com.process = p;

				i = find(m_rmanager.m_waitingQueue[it->first - 1].begin(), m_rmanager.m_waitingQueue[it->first - 1].end(), com);
				if (i != m_rmanager.m_waitingQueue[it->first - 1].end())
					m_rmanager.m_waitingQueue[it->first - 1].erase(i);
			}
		}
		delete p;
		return;

	}

	//for(vector<PCB*>::iterator s = sons.begin(); s != sons.end(); ++s)
	//	killTree(*s);//�ݹ�ɾ��
	size_t size = sons.size();
	for (int i = 0;i != size; i++)
	{
		killTree(sons[0]);
	}
	killTree(p);
}