#include "stdafx.h"
#include "SqLite3MemAloc.h"

sqlite3_mem_methods CSqLite3MemAloc::m_OldMem;
sqlite3_mem_methods CSqLite3MemAloc::m_Mem;

__TLS  CSqLite3MemAloc::SHips* CSqLite3MemAloc::m_pH=NULL;

CSqLite3MemAloc::CSqLite3MemAloc(void)
{
	//ASSERT(m_nIndTls==0);
	//m_nIndTls=TlsAlloc();
}

CSqLite3MemAloc::~CSqLite3MemAloc(void)
{
}
void CSqLite3MemAloc::Init()
{
  // return;
	ASSERT(m_OldMem.pAppData==0);
	sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m_OldMem);

	m_Mem.xMalloc=Malloc;
	m_Mem.xFree=Free;
	m_Mem.xRealloc=Realloc;
	m_Mem.xSize=Size;
	m_Mem.xRoundup=Roundup;
	m_Mem.xInit=Init;
	m_Mem.xShutdown=Shutdown;
	m_Mem.pAppData=m_OldMem.pAppData;

//#ifndef _DEBUG  //� ������ ������� �������� ������ � ������ ������ �����������
	sqlite3_config(SQLITE_CONFIG_MALLOC, &m_Mem);
//#endif
}
void CSqLite3MemAloc::Close()
{
	sqlite3_config(SQLITE_CONFIG_MALLOC, &m_OldMem);
}

void* CSqLite3MemAloc::Malloc(int n)         /* Memory allocation function */
{
	if(m_pH==NULL)
	{
		m_pH=new SHips;
	}

	return m_pH->Malloc(n);


}
void CSqLite3MemAloc::Free(void* p)          /* Free a prior allocation */
{
	if(m_pH)
		m_pH->Free(p);

}
void* CSqLite3MemAloc::Realloc(void* p,int n)  /* Resize an allocation */
{
	return m_pH->Realloc(p,n);
}

int CSqLite3MemAloc::Size(void* p)           /* Return the size of an allocation */
{
	return m_pH->Size(p);
}

int CSqLite3MemAloc::Roundup(int n)          /* Round up request size to allocation size */
{
	return (((n)+7)&~7);
}

int CSqLite3MemAloc::Init(void* p)           /* Initialize the memory allocator */
{
	return SQLITE_OK;
}
void CSqLite3MemAloc::Shutdown(void*p)      /* Deinitialize the memory allocator */
{
	m_OldMem.xShutdown(p);
}

//////////////////////////////////////////////////////////

//struct SHips
CSqLite3MemAloc::SHips::SHips()
{
	ZeroMemory(m_hip,sizeof(m_hip));
}
CSqLite3MemAloc::SHips::~SHips()
{
	for(unsigned i=0;i<_countof(m_hip);++i)
		delete m_hip[i];
}
//	SHip* m_hip[9]; // ������ ���������� ������ ������ m_hip[0] -> ��� �� 64 �����,m_hip[1]- 128 ���� � ��

void* CSqLite3MemAloc::SHips::Malloc(int n)
{
	return GetHip(n)->Malloc(n);
}
void CSqLite3MemAloc::SHips::Free(void* p)
{
	GetHip(Size(p))->Free(p);
}
void* CSqLite3MemAloc::SHips::Realloc(void* p,int n)
{
	size_t szOld=Size(p);
	if(szOld==size_t(n))
		return p;

	SHip* hN=GetHip(n);
	SHip* hO=GetHip(szOld);
	if(hN==hO)
	{
		//������ ������ �������
		SShunk &pSh=((SShunk *)p)[-1];
		pSh.nSize=n;
		return p;
	}

	void* pNew=Malloc(n);
	memcpy(pNew,p,MIN(n,(int)szOld));
	Free(p);
	return pNew;
}
size_t CSqLite3MemAloc::SHips::Size(void* p)
{
	if(!p)
		return 0;
	SShunk &pSh=((SShunk *)p)[-1];
	return pSh.nSize;
}

CSqLite3MemAloc::SHip* CSqLite3MemAloc::SHips::GetHip(int n)
{
	//int pos=0;
	/*
	static const int p[13]=
	{
	    64-sizeof(SShunk),
	    (64<<1)-sizeof(SShunk),

	}
	*/

	//for(;int((64<<pos)-sizeof(SShunk)) < n;++pos);
	int z;

	if(int((64<<7)-sizeof(SShunk)) >=n)
	{

        if(int((64<<3)-sizeof(SShunk)) >= n)
        {
                if(int((64<<1)-sizeof(SShunk)) >= n)
                { //0 ,1
                    z=int(64-sizeof(SShunk)) < n;

                }
                else
                { //2 3
                    z= 2 + (int((64<<2)-sizeof(SShunk)) < n);

                }

        }
        else
        {// 4 5 6 7

                if(int((64<<5)-sizeof(SShunk)) >= n)
                { //4 5
                    z=4+(int((64<<4)-sizeof(SShunk)) < n);

                }
                else
                { //6 7
                    z= 6 + (int((64<<6)-sizeof(SShunk)) < n);

                }
        }
	}
	else
	{
        if(int((64<<11)-sizeof(SShunk)) >= n)
        {//8 9 10 11
                if(int((64<<9)-sizeof(SShunk)) >= n)
                { //8 9
                    z=8 + (int((64<<8)-sizeof(SShunk)) < n);

                }
                else
                { //10 11
                    z= 10 + (int((64<<10)-sizeof(SShunk)) < n);

                }

        }
        else
        {// 12 13

                if(int((64<<13)-sizeof(SShunk)) >= n)
                { //12 13
                    z=12+(int((64<<12)-sizeof(SShunk)) < n);

                }
                else
                { //14 ...
                    ASSERT(false);

                }
        }

	}
	//ASSERT(z == pos);

	//ASSERT(pos < _countof(m_hip));
	//pos=z;

	if(m_hip[z]==NULL)
		m_hip[z]=new SHip(64<<z);

	return m_hip[z];

}
////////////////////////////////////////

CSqLite3MemAloc::SHip::SHip(int SizeShunk,int nVer)
:m_nVer(nVer)
,m_SizeShunk( SizeShunk)
,m_CountShunk(MAX(16,nVer*65536/SizeShunk))
,m_nPosNext(0)
,m_pNext(NULL)
,m_pBuf(new __int8[m_CountShunk*m_SizeShunk])
{
	//��������������
	__int8 *pb=m_pBuf;
	for(unsigned i=1;i<m_CountShunk;++i,pb+=SizeShunk)
		((SShunk*)pb)->nPosNext=i;
   ((SShunk*)pb)->nPosNext=-1; //��������� �� ���� �� ���������


}
CSqLite3MemAloc::SHip::~SHip()
{
	delete m_pNext;
	delete []m_pBuf;
}
/*
size_t  m_SizeShunk; //������ �������
size_t  m_CountShunk; //���������� �������� � � ���� ����
size_t m_nPosNext; //������� ������� �� ��������
SHip * m_pNext; //������� ���������� � ���� ����
		void*  m_pBuf; //��������������� ����� ������
*/
void* CSqLite3MemAloc::SHip::Malloc(int n)
{
	if(m_nPosNext!=size_t(-1))
	{

		SShunk* sh=(SShunk*)(m_pBuf + m_nPosNext*m_SizeShunk);
		m_nPosNext=sh->nPosNext;
		sh->nSize=n;
		return sh+1;
	}
	if(m_pNext==NULL)
		m_pNext= new SHip(m_SizeShunk, m_nVer*2);

	void* ret = m_pNext->Malloc(n);

	swap();

	return ret;
}
void CSqLite3MemAloc::SHip::swap()
{
	std::swap(m_CountShunk,m_pNext->m_CountShunk);
	std::swap(m_nPosNext,m_pNext->m_nPosNext);
	std::swap(m_pBuf,m_pNext->m_pBuf);
}

void CSqLite3MemAloc::SHip::Free(void* p)
{
	if(p> m_pBuf && p < m_pBuf+m_CountShunk*m_SizeShunk)
	{
		SShunk* sh=&((SShunk*)p)[-1];
		int n=( (__int8*)sh  - m_pBuf)/ m_SizeShunk;
		ASSERT( ( (__int8*)sh  - m_pBuf)% m_SizeShunk == 0);
		sh->nPosNext=m_nPosNext;
		m_nPosNext=n;

	}
	else if(m_pNext)
	{
		m_pNext->Free(p);
		swap();
	}
	else
	{
		ASSERT(FALSE);

	}

}
