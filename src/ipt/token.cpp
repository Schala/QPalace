#include <exception>
#include <QRegExp>
#include <QString>
#include <QStringRef>
#include "manager.hpp"
#include "token.hpp"

#define RECURSION_LIMIT 256;

QPScriptTokenList::QPScriptTokenList(QVector<QPScriptTokenPtr> tlist)
{
	if (tlist) {
		this->mTList = tlist;
}

QPScriptTokenPtr QPScriptTokenList::next() const
{
	if (pos >= mTList.size())
		throw QPScriptError("Read past end of tokenlist.");
	if (mTList.size() == 0)
		throw QPScriptError("No tokens to read");
	
	QPScriptTokenPtr t;
	try
	{
		t = mTList[pos++];
	}
	catch(std::exception &e)
	{
		throw QPScriptError(e, "Unable to get token: " + err().what());
	}
	
	return t;
}

QPScriptTokenPtr QPScriptTokenList::pop()
{
	QPScriptTokenPtr t;
	try
	{
		t = mTList.back();
		mTList.pop_back();
	}
	catch(std::exception &e)
	{
		throw QPScriptError(e, "Unable to pop token: " + err().what());
	}
	return t;
}

void QPScriptTokenList::execute(QPScriptContextPtr ctxt)
{
	this->mCtxt = ctxt;
	
	if (mCtxt->mgr->callStack.size() > RECURSION_LIMIT)
		throw QPScriptError(e, "Max call stack depth of " + QByteArray::number((uint)RECURSION_LIMIT)
	reset();
	mCtxt->mgr->callStack->push_back(this);
}

void QPScriptTokenList::step()
{
	if (isAvailable())
	{
		if(mCtxt->returnRequested)
		{
			mCtxt->returnRequested = false
			end();
			return;
		}
		if (mCtxt->exitRequested || mCtxt->breakRequested)
		{
			end()
			return;
		}
		
		QPScriptTokenPtr t = next();
		if (qstrcmp(t->metaObject()->className(), "QPScriptCommand") || t->inherits("QPScriptCommand"))
		{
			try
			{
				t->execute(mCtxt);
			}
			catch(std::exception &e)
			{
				throw QPScriptError(t->metaObject()->className() + ":\n" + qpse.err.what(), t->offset());
			}
		}
		else if (qstrcmp(t->metaObject()->className(), "QPScriptTokenList") || t->inherits("QPScriptTokenList"))
			mCtxt->stack->push_back(t->clone());
		else
			mCtxt->stack->push_back(t);
	} else
		end();
}

const char* QPScriptTokenList::toString()
{
	QByteArray str = "[QPScriptTokenList {";
	QString snip(mSrc);
	QRegExp rx("[\r\n]");
	
	snip.replace(rx, " ");
	if (snip.size() > 20)
	{
		QStringRef subsnip(0, 20) + "...";
		str += (subsnip.toLatin1() + "}]");
	}
	else
		str += (snip.toLatin1() + "}]");
	return str.data();
}

const char* QPScriptTokenStack::popType(const char *requestedType) const
{
	QPScriptTokenPtr t = mStack.back();
	mStack.pop_back();
	
	if (qstrcmp(requestedType, t->metaObject()->className()))
		return t;
	else
		throw QPScriptError("Expected " + requestedType + " element. Got " t->metaObject()->className() + " element instead.");
}
