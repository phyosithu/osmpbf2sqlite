#pragma once

class CSqLite3MemAloc
{
public:
	CSqLite3MemAloc(void);
	~CSqLite3MemAloc(void);

	static sqlite3_mem_methods m_OldMem;
	static sqlite3_mem_methods m_Mem;

	void Init();
	void Close();

	static  void* Malloc(int n);         /* Memory allocation function */
	static  void Free(void* p);          /* Free a prior allocation */
    static  void* Realloc(void* p,int n);  /* Resize an allocation */
	static  int Size(void* p);           /* Return the size of an allocation */
	static  int Roundup(int n);          /* Round up request size to allocation size */
	static int Init(void* p);           /* Initialize the memory allocator */
	static void Shutdown(void*p);      /* Deinitialize the memory allocator */



	//�������
	struct SShunk
	{
		//union
		//{
			size_t nSize; //������ �����
			size_t nPosNext; //������� ���������� �� ��������
		//};
	};

	//������� ����������� �������
	struct SHip
	{
		SHip(int SizeShunk,int nVer=1);
		~SHip();
		int m_nVer;  //����� ��� ���������� ��������� ������
		size_t  m_SizeShunk; //������ �������
		size_t  m_CountShunk; //���������� �������� � � ���� ����
		size_t m_nPosNext; //������� ������� �� ��������
		SHip * m_pNext; //������� ���������� � ���� ����
		__int8*  m_pBuf; //��������������� ����� ������

		void* Malloc(int n);
		void Free(void* p);
	private:
		void swap();
	};

	struct SHips
	{
		SHips();
		~SHips();

		SHip* m_hip[13]; // ������ ���������� ������ ������ m_hip[0] -> ��� �� 64 �����,m_hip[1]- 128 ���� � ��

		void* Malloc(int n);
		void Free(void* p);
		void* Realloc(void* p,int n);
		size_t Size(void* p);

		SHip* GetHip(int n);
	};


    static __TLS  SHips* m_pH;
};
