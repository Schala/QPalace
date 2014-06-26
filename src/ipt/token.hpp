#ifndef _IPT_TOKEN_H
#define _IPT_TOKEN_H

#include <QByteArray>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QVector>

class QPScriptToken: virtual public QObject, virtual public QSharedData {
	Q_OBJECT
public:
	QPScriptToken(): mOffs(-1) {}
	
	inline int offset() const {
		return mOffs;
	}
protected:
	int mOffs;
};

typedef QSharedDataPointer<QPScriptToken> QPScriptTokenPtr

class QPScriptTokenList: virtual public QPScriptToken, virtual public QPScriptRunnable {
	Q_OBJECT
public:
	QPScriptTokenList(QVector<QPScriptTokenPtr> tlist = QVector<QPScriptTokenPtr>):
		mOffsetCompensation(0), mRunning(false), mPos(0);
	QPScriptTokenList(const QPScriptTokenList *other):
		offsetCompensation(0), mRunning(false), mPos(0), mSrc(other->source()),
		offset(other->offset);
	
	inline bool isRunning() const {
		return mRunning;
	};
	
	inline void isRunning(bool value) {
		mRunning = value;
	};
	
	inline void reset() {
		mPos = 0;
		mRunning = true;
	};
	
	inline QPScriptTokenPtr current() const {
		return mPos < mTList.size() ? mTList[mPos] : nullptr;
	};
	
	QPScriptTokenPtr next() const;
	
	inline bool isAvailable() const {
		return (bool)(mPos < mTList.size());
	};
	
	inline unsigned size() const {
		return mTList.size();
	};
	
	inline void push(QPScriptTokenPtr t, int offset = -1) {
		t->mOffs = offset;
		mTList.push_back(t);
	};
	
	QPScriptTokenPtr pop();
	
	void execute(QPScriptContextPtr ctxt);
	
	inline void end() {
		mRunning = false;
	};
	
	inline QByteArray& source() const {
		return mSrc;
	}
	
	void step();
	QByteArray& toString();
protected:
	bool mRunning;
	QByteArray mSrc;
	int mOffsetCompensation;
	QPScriptContextPtr mCtxt;
private:
	QVector<QPScriptTokenPtr> mTList;
	uint mPos;
};

class QPScriptTokenStack {
public:
	inline uint size() const {
		return stack.size();
	};
	
	QPScriptTokenPtr pop();
	void push(QPScriptTokenPtr t);
	QPScriptTokenPtr operator[](uint mPos) const;
	void copy();
	QByteArray& popType(const QByteArray &requestedType) const;
protected:
	QVector<QPScriptTokenPtr> mStack;
};

#endif // _IPT_TOKEN_H
