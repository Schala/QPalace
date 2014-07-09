#ifndef _IPT_TOKEN_H
#define _IPT_TOKEN_H

#include <QByteArray>
#include <QSharedPointer>
#include <QtGlobal>
#include <QVector>

class QPScriptToken: public QObject
{
	Q_OBJECT
public:
	QPScriptToken(): mOffs(-1) {}
	inline qint32 offset() const { return mOffs; }
protected:
	qint32 mOffs;
};

typedef QSharedPointer<QPScriptToken> QPScriptTokenPtr

class QPScriptTokenList: public QPScriptToken, public QPScriptRunnable
{
	Q_OBJECT
public:
	QPScriptTokenList(QVector<QPScriptTokenPtr> tlist = QVector<QPScriptTokenPtr>()):
		mOffsetCompensation(0), mRunning(false), mPos(0);
	QPScriptTokenList(const QPScriptTokenList *other):
		offsetCompensation(0), mRunning(false), mPos(0), mSrc(other->source()),
		offset(other->offset);
	inline bool isRunning() const { return mRunning; }
	inline void isRunning(bool value) { mRunning = value; }
	
	inline void reset()
	{
		mPos = 0;
		mRunning = true;
	}
	
	inline QPScriptTokenPtr current() const { return mPos < mTList.size() ? mTList[mPos] : nullptr; }
	QPScriptTokenPtr next() const;
	inline bool isAvailable() const { return (bool)(mPos < mTList.size()); }
	inline quint32 size() const { return mTList.size(); }
	
	inline void push(QPScriptTokenPtr t, qint32 offset = -1)
	{
		t->mOffs = offset;
		mTList.push_back(t);
	}
	
	QPScriptTokenPtr pop();
	void execute(QPScriptContextPtr ctxt);
	inline void end() { mRunning = false; }
	inline QByteArray& source() const { return mSrc; }
	void step();
	const char* toString();
protected:
	bool mRunning;
	QByteArray mSrc;
	qint32 mOffsetCompensation;
	QPScriptContextPtr mCtxt;
private:
	QVector<QPScriptTokenPtr> mTList;
	quint32 mPos;
};

class QPScriptTokenStack
{
public:
	inline quint32 size() const { return stack.size(); }
	QPScriptTokenPtr pop();
	void push(QPScriptTokenPtr t);
	QPScriptTokenPtr operator[](quint32 mPos) const;
	void copy();
	const char* popType(const char *requestedType) const;
protected:
	QVector<QPScriptTokenPtr> mStack;
};

#endif // _IPT_TOKEN_H
