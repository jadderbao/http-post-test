#include "http_script_engine.h"
#include <QScriptClassPropertyIterator>
#include <QScriptContext>
#include <QScriptEngine>

#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>

#include <stdlib.h>

#include "code/ucenter_authcode.h"
#include "code/md5.h"

Q_DECLARE_METATYPE(http_data_ptr)
Q_DECLARE_METATYPE(http_data_list)
Q_DECLARE_METATYPE(http_data_list*)
Q_DECLARE_METATYPE(post_data_list_class*)

//post_data_utils

code_tool::code_tool(QObject *parent /*= 0*/)
	:QObject(parent)
{

}

code_tool::~code_tool()
{

}

QString code_tool::ucenter_encode_url_query(const QString& key, 
	http_data_list* datas, int type)
{
	QString str;
	for (auto it = datas->begin(); it != datas->end(); ++it){
		http_data_ptr pdp = *it;
		if (pdp->post_type() & type){
			str.append(pdp->key().toUtf8().toPercentEncoding());
			str.append('=');
			str.append(pdp->value().toUtf8().toPercentEncoding());
			str.append('&');
		}
	}

	if (!str.isEmpty()){
		str.chop(1);
	}

	return ucenter_encode(key, str);
}

QString code_tool::ucenter_encode(const QString& key, const QString& data)
{
	return QString::fromStdString(ucenter_authcode_encode(data.toStdString(), key.toStdString()));
}

QString code_tool::ucenter_decode(const QString& key, const QString& data)
{
	return QString::fromStdString(ucenter_authcode_decode(data.toStdString(), key.toStdString()));
}

QString code_tool::md4(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md4).toHex();
}

QString code_tool::md5(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5).toHex();
}

QString code_tool::sha1(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha1).toHex();
}

QString code_tool::sha224(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha224).toHex();
}

QString code_tool::sha256(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256).toHex();
}

QString code_tool::sha384(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha384).toHex();
}

QString code_tool::sha512(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha512).toHex();
}

QString code_tool::sha3_224(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha3_224).toHex();
}

QString code_tool::sha3_256(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha3_256).toHex();
}

QString code_tool::sha3_384(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha3_384).toHex();
}

QString code_tool::sha3_512(const QString& data)
{
	return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha3_512).toHex();
}

QString code_tool::url_encode(const QString& data)
{
	return data.toUtf8().toPercentEncoding();
}

QString code_tool::url_decode(const QString& data)
{
	return QByteArray::fromPercentEncoding(data.toUtf8());
}

QString code_tool::base64_encode(const QString& data)
{
	return data.toUtf8().toBase64();
}

QString code_tool::base64_decode(const QString& data)
{
	return QByteArray::fromBase64(data.toUtf8());
}

//post_data_list_property_iterator

class post_data_list_property_iterator : public QScriptClassPropertyIterator
{
public:
	post_data_list_property_iterator(const QScriptValue &object);
	~post_data_list_property_iterator();

	bool hasNext() const;
	void next();

	bool hasPrevious() const;
	void previous();

	void toFront();
	void toBack();

	QScriptString name() const;
	uint id() const;

private:
	int m_index;
	int m_last;
};


post_data_list_property_iterator::post_data_list_property_iterator(const QScriptValue &object)
: QScriptClassPropertyIterator(object)
{
	toFront();
}

post_data_list_property_iterator::~post_data_list_property_iterator()
{
}

bool post_data_list_property_iterator::hasNext() const
{
	http_data_list *pdli = qscriptvalue_cast<http_data_list*>(object().data());
	return m_index < pdli->size();
}

void post_data_list_property_iterator::next()
{
	m_last = m_index;
	++m_index;
}

bool post_data_list_property_iterator::hasPrevious() const
{
	return (m_index > 0);
}

void post_data_list_property_iterator::previous()
{
	--m_index;
	m_last = m_index;
}

void post_data_list_property_iterator::toFront()
{
	m_index = 0;
	m_last = -1;
}

void post_data_list_property_iterator::toBack()
{
	http_data_list *pdli = qscriptvalue_cast<http_data_list*>(object().data());
	m_index = pdli->size();
	m_last = -1;
}

QScriptString post_data_list_property_iterator::name() const
{
	return object().engine()->toStringHandle(QString::number(m_last));
}

uint post_data_list_property_iterator::id() const
{
	return m_last;
}

//post_data_list_prototype

post_data_list_prototype::post_data_list_prototype(QObject *parent /*= 0*/)
	:QObject(parent)
{

}

post_data_list_prototype::~post_data_list_prototype()
{

}

http_data_list * post_data_list_prototype::this_post_data_list() const
{
	return qscriptvalue_cast<http_data_list*>(thisObject().data());
}

QString post_data_list_prototype::to_url_encode_string() const
{
	return this_post_data_list()->to_url_encode_string();
}

http_data_list post_data_list_prototype::get_post_datas(int post_type, int auth_type) const
{
	return this_post_data_list()->get_post_datas(post_type, auth_type);
}

post_data_list_class::post_data_list_class(QScriptEngine *engine)
	: QObject(engine), QScriptClass(engine)
{
	qScriptRegisterMetaType<http_data_list>(engine, toScriptValue, fromScriptValue);

	length = engine->toStringHandle(QLatin1String("length"));

	proto = engine->newQObject(new post_data_list_prototype(this),
		QScriptEngine::QtOwnership,
		QScriptEngine::SkipMethodsInEnumeration
		| QScriptEngine::ExcludeSuperClassMethods
		| QScriptEngine::ExcludeSuperClassProperties);
	QScriptValue global = engine->globalObject();
	proto.setPrototype(global.property("Object").property("prototype"));

	ctor = engine->newFunction(construct, proto);
	ctor.setData(engine->toScriptValue(this));
}

post_data_list_class::~post_data_list_class()
{

}

QScriptValue post_data_list_class::newInstance()
{
	return newInstance(http_data_list());
}

QScriptValue post_data_list_class::newInstance(const http_data_list &pdl)
{
	QScriptValue data = engine()->newVariant(QVariant::fromValue(pdl));
	return engine()->newObject(this, data);
}

QScriptClass::QueryFlags post_data_list_class::queryProperty(const QScriptValue &object,
	const QScriptString &name, QueryFlags flags, uint *id)
{
	http_data_list *ba = qscriptvalue_cast<http_data_list*>(object.data());
	if (!ba)
		return 0;
	if (name == length) {
		return flags;
	}
	else {
		bool isArrayIndex;
		qint32 pos = name.toArrayIndex(&isArrayIndex);
		if (!isArrayIndex)
			return 0;
		*id = pos;
		if ((flags & HandlesReadAccess) && (pos >= ba->size()))
			flags &= ~HandlesReadAccess;
		return flags;
	}
}

QScriptValue post_data_list_class::property(const QScriptValue &object, 
	const QScriptString &name, uint id)
{
	http_data_list *pdli = qscriptvalue_cast<http_data_list*>(object.data());
	if (!pdli)
		return QScriptValue();
	if (name == length) {
		return pdli->length();
	}
	else {
		qint32 pos = id;
		if ((pos < 0) || (pos >= pdli->size()))
			return QScriptValue();
		return engine()->newVariant(QVariant::fromValue(pdli->at(pos)));
	}
	return QScriptValue();
}

void post_data_list_class::setProperty(QScriptValue &object, const QScriptString &name,
	uint id, const QScriptValue &value)
{
	http_data_list *pdli = qscriptvalue_cast<http_data_list*>(object.data());
	if (!pdli)
		return;

	qint32 pos = id;
	if (pos < 0)
		return;

	if (pdli->size() <= pos)
		return;

	(*pdli)[pos] = value.toVariant().value<http_data_ptr>();
}

QScriptValue::PropertyFlags post_data_list_class::propertyFlags(const QScriptValue &object, 
	const QScriptString &name, uint id)
{
	if (name == length) {
		return QScriptValue::Undeletable
			| QScriptValue::SkipInEnumeration;
	}
	return QScriptValue::Undeletable;
}

QScriptClassPropertyIterator * post_data_list_class::newIterator(const QScriptValue &object)
{
	return new post_data_list_property_iterator(object);
}

QString post_data_list_class::name() const
{
	return QLatin1String("post_data_list");
}

QScriptValue post_data_list_class::prototype() const
{
	return proto;
}

QScriptValue post_data_list_class::constructor()
{
	return ctor;
}

QScriptValue post_data_list_class::construct(QScriptContext *ctx, QScriptEngine *eng)
{
	post_data_list_class *cls = qscriptvalue_cast<post_data_list_class*>(ctx->callee().data());
	if (!cls)
		return QScriptValue();

	QScriptValue arg = ctx->argument(0);
	if (arg.instanceOf(ctx->callee()))
		return cls->newInstance(qscriptvalue_cast<http_data_list>(arg));

	int size = arg.toInt32();
	return cls->newInstance();
}

QScriptValue post_data_list_class::toScriptValue(QScriptEngine *eng, const http_data_list &pdl)
{
	QScriptValue ctor = eng->globalObject().property("post_data_list");
	post_data_list_class *cls = qscriptvalue_cast<post_data_list_class*>(ctor.data());
	if (!cls)
		return eng->newVariant(QVariant::fromValue(pdl));
	return cls->newInstance(pdl);
}

void post_data_list_class::fromScriptValue(const QScriptValue &obj, http_data_list &pdl)
{
	pdl = qvariant_cast<http_data_list>(obj.data().toVariant());
}

http_script_engine::http_script_engine(QObject *parent)
	: QScriptEngine(parent)
	, _code_tool(new code_tool(this))
{
	initialize_default();
}

http_script_engine::~http_script_engine()
{

}

void http_script_engine::initialize_default()
{
	QScriptValue global_object = globalObject();
	global_object.setProperty("load",newFunction(loadScripts, 1));

	post_data_list_class *pdlc = new post_data_list_class(this);
	global_object.setProperty("post_data_list", pdlc->constructor());

	setDefaultPrototype(qMetaTypeId<http_data_list*>(), pdlc->newInstance());

	global_object.setProperty("code_tool", newQObject(_code_tool));
}


QScriptValue http_script_engine::loadScripts(QScriptContext *context, QScriptEngine *engine)
{
	for (int i = 0; i < context->argumentCount(); ++i) {
		QString fileName = context->argument(i).toString();
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly))
			return context->throwError(QString::fromLatin1("could not open %0 for reading").arg(fileName));
		QTextStream ts(&file);
		QString contents = ts.readAll();
		file.close();
		QScriptContext *pc = context->parentContext();
		context->setActivationObject(pc->activationObject());
		context->setThisObject(pc->thisObject());
		QScriptValue ret = engine->evaluate(contents);
		if (engine->hasUncaughtException())
			return ret;
	}
	return engine->undefinedValue();
}

