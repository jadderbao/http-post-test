#ifndef HTTP_SCRIPT_H
#define HTTP_SCRIPT_H

#include <QObject>
#include <QScriptClass>
#include <QScriptable>
#include <QScriptValue>
#include <QScriptEngine>

#include "http_data.h"

QT_BEGIN_NAMESPACE
class QScriptContext;
QT_END_NAMESPACE

class code_tool : public QObject
{
	Q_OBJECT

public:
	code_tool(QObject *parent = 0);
	~code_tool();

public slots:
	QString ucenter_encode_url_query(const QString& key, http_data_list* datas, int type);
	QString ucenter_encode(const QString& key, const QString& data);
	QString ucenter_decode(const QString& key, const QString& data);
	QString md4(const QString& data);
	QString md5(const QString& data);
	QString sha1(const QString& data);
	QString sha224(const QString& data);
	QString sha256(const QString& data);
	QString sha384(const QString& data);
	QString sha512(const QString& data);
	QString sha3_224(const QString& data);
	QString sha3_256(const QString& data);
	QString sha3_384(const QString& data);
	QString sha3_512(const QString& data);
	QString url_encode(const QString& data);
	QString url_decode(const QString& data);
	QString base64_encode(const QString& data);
	QString base64_decode(const QString& data);
};

class post_data_list_prototype : public QObject, public QScriptable
{
	Q_OBJECT

public:
	post_data_list_prototype(QObject *parent = 0);
	~post_data_list_prototype();

public slots:
	QString to_url_encode_string() const;
	http_data_list get_post_datas(int post_type, int auth_type) const;
private:
	http_data_list *this_post_data_list() const;
};

class post_data_list_class : public QObject, public QScriptClass
{
	Q_OBJECT

public:
	post_data_list_class(QScriptEngine *engine);
	~post_data_list_class();

	QScriptValue constructor();
	QScriptValue newInstance();
	QScriptValue newInstance(const http_data_list &pdl);

	QueryFlags queryProperty(const QScriptValue &object,
		const QScriptString &name,
		QueryFlags flags, uint *id);

	QScriptValue property(const QScriptValue &object,
		const QScriptString &name, uint id);

	void setProperty(QScriptValue &object, const QScriptString &name,
		uint id, const QScriptValue &value);

	QScriptValue::PropertyFlags propertyFlags(
		const QScriptValue &object, const QScriptString &name, uint id);

	QScriptClassPropertyIterator *newIterator(const QScriptValue &object);

	QString name() const;

	QScriptValue prototype() const;

private:
	static QScriptValue construct(QScriptContext *ctx, QScriptEngine *eng);
	static QScriptValue toScriptValue(QScriptEngine *eng, const http_data_list &pdl);
	static void fromScriptValue(const QScriptValue &obj, http_data_list &pdl);

	QScriptString length;
	QScriptValue proto;
	QScriptValue ctor;
};

class http_script_engine : public QScriptEngine
{
	Q_OBJECT

public:
	http_script_engine(QObject *parent);
	~http_script_engine();
public:
	static QScriptValue loadScripts(QScriptContext *context, QScriptEngine *engine);

private:
	void initialize_default();

private:
	code_tool *_code_tool;
};

#endif // HTTP_SCRIPT_H
