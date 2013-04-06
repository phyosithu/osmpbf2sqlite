#pragma once

class CThreadUnit;


//����� ����������� ������� ����������� �������
class CThreadManager
{
public:
	//������� ��� ������� �������
	//nThread ���������� ������� ������� 0 - ������� ������� ������� ����
	CThreadManager(unsigned nThread=0);

	~CThreadManager(void);

	/*������������ ������� �������

	pThread - ������ ��������������� ������������ ����������� ���� ������
	nGrupId - ������������� ���������� uheggs �������. �� ����������� ���������
	nGrupIdStartAfter - ������������� ������ ������� ����� ���������� ������� ��������� ���� �����. 0 ��������� �����
	nGrupIdWaitKey - ������ �������� � ������� �����. 0 ��������� �����. ����� ���� ��������� ���� ������

	���������� ���������� ������������� �������

	*/

	typedef unsigned GrupId;  //������������� ������ �������
	typedef const CThreadUnit* TaskId;  //���������� ������������� �������


	//��������� ������ pThread � ������� �����
	//������ ����� ������������� ������ gidMy � ������ ����������� �����
	// ����������� nCount ����� �� ������  gidAfter
	TaskId BeginTaskG(CThreadUnit* pThread,GrupId gidMy,GrupId gidAfter,int nCount,int nCountCoin=1);

	//��������� ������ pThread � ������� �����
	//������ ����� ������������� ������ gidMy � ������ ����������� ����� ������ pThreadAfter
	TaskId BeginTaskT(CThreadUnit* pThread,GrupId gidMy,CThreadUnit* pThreadAfter,int nCountCoin=1);

	struct STasksGrup
	{
		bool m_bGrup;
		union
		{
			CThreadUnit* m_pThreadAfter;
			struct
			{
				GrupId m_gidAfter;
				int m_nCount;
			};
		};
	};


	//��������� ������ pThread � ������� �����
	//������ ����� ������������� ������ gidMy � ������ ����������� �����
	// ����������� ��� ������ ��������� � ������� pTasks. ������ �������� nCountTassks �����
	TaskId BeginTaskComplex(CThreadUnit* pThread,GrupId gidMy,const STasksGrup* pTasks,int nCountTassks,int nCountCoin=1);



	//��������� �������
	enum ETaskInfo
	{
		etiUnknown,  //����������
		etiWaitPrev,     //������� ���������� ���������� �����
		etiWaitThread,    //������� ���������� ������
		etiWork,     //�����������
		etiReadyUnknown,  //������ ���������� � � ����� �� ���
		etiReady,  //������ ���������� � ����������� � ������� ��� ����������� ���������
	};

	//������ ��� ���������� � �������
	ETaskInfo GetInfo(TaskId ti)const;

	//������� ������ ���� ���� ��� �����������
	void KillTask(TaskId ti);


	//�������� ���������� ���������� ����� �� ������ giW ���� 0 �� ���� ����
	void Wait(GrupId giW,int nCount=0);



	//void SetThreadPriority(int nPriority);

	static unsigned GetCountKernels(); // ������� � ������� ����

	size_t GetCountThread()const
		{return m_arHandle.size();} // ������� ������� ������������ ������ � ������ ����� �������

private:
	mutable CComAutoCriticalSection m_cs;

	struct STask
	{
		TaskId m_taskId;  //���� ������
		GrupId m_Grup;  //������
		int m_nCountCoin;   //�������� ������� ����������� ����� ���� ������
		int m_nCountCoinReady;   //�������� ����� ��� �� ����������
		ETaskInfo m_etiState;//���������

		std::vector<STasksGrup> m_arDefsTasks; //������ ������������
		std::vector<TaskId> m_arTasks; //������ ������� ����� ������ ��� ������ ���������� ���� ������
		void ChekStateWP();

	};

	typedef std::map<TaskId,STask> CTasks;
	CTasks m_arTask;

	STask &GetTask(TaskId tid);
	CTasks::iterator GetTaskIterator(TaskId tid);



	struct SThread
	{
		SThread():m_taskWork(0){}
		boost::thread::id m_ThreadId;
		TaskId m_taskWork;
		CThreadManager *m_ptm;
		class CMessageQueue
		{
        public:
            CMessageQueue():m_nWrite(0),m_nRead(0),m_arMessage(8){}

            bool PostThreadMessage(unsigned message);
            //����� �� �������  ��� WM_QUIT �� ���������� �����
            bool GetMessage(unsigned *pMessage);

            private:
            int m_nWrite; //�������� ������
            int m_nRead;//�������� ������
            std::vector<unsigned> m_arMessage;
            boost::condition_variable m_cond; //�������
            boost::mutex m_mut; //��������

		};
		 CMessageQueue m_MessageQueue;
		 void operator()();

		//������� ������ �� ���������� �����  ������ � ���� GrupId. ���� ����� �� ==0 ��
		//��������� ��������
		// ���������� ������ ���� ��������
		bool AddTaskAfter(GrupId ngId, TaskId tId);
	};


	std::vector<SThread*> m_arThreads;
//	std::vector<HANDLE> m_arHandle;
	std::vector<boost::thread *> m_arHandle;

	static const UINT TM_START=WM_USER+1;
	static const UINT TM_INIT= WM_USER+2;
	static DWORD WINAPI Thread(  LPVOID lpThreadParameter  );
	friend class CWait;


	void WakeUpThread();


};



//������ ����� ��� �������� �����
class CThreadUnit
{
public:
	CThreadUnit():m_GrupId(0),m_pThreadManager(NULL){}
	virtual ~CThreadUnit(){};

	virtual void Start(CThreadUnit** pTasks, int countTasks)=0;


	CThreadManager::TaskId GetTaskId()const { return this;}
	CThreadManager::GrupId GetGrupId()const { return m_GrupId;}
	CThreadManager* GetThreadManager()const { return m_pThreadManager;}

private:
	CThreadManager::GrupId m_GrupId;
	CThreadManager* m_pThreadManager;
	friend class CThreadManager;


};

/*
inline bool CThreadManager::STask::operator<(const CThreadManager::STask::STask& t)const
{	return m_taskId<t.m_taskId;
}
*/
inline CThreadManager::STask &CThreadManager::GetTask(CThreadManager::TaskId tid)
{

    CTasks::iterator ti=GetTaskIterator(tid);
	return ti->second;
}
