#ifndef _PIESF_h_
#define _PIESF_h_

class CPIESF
{
public:
    CPIESF();
    ~CPIESF();
    
private:
    bool m_bIsContinue;
    
public:
    bool isActive(void);
    
public:
    static CPIESF& getInstance( void );
    void run(void);
};

#endif /* _PIESF_h_ */
