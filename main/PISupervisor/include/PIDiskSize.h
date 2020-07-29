#ifndef	_PIDISKSIZE_H
#define	_PIDISKSIZE_H

class CPIDiskSize {
	public:
		CPIDiskSize();
		virtual ~CPIDiskSize();

	public:
		double totalBytes;
		double freeBytes;
		double usedBytes;

	public :
		void clear(void);
		bool getSystemDiskSize(void);
		void getUsedBytes(void);
};
#endif // #ifndef _PIDISKSIZE_H
