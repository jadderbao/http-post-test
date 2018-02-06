#ifndef POST_DATA_H
#define POST_DATA_H

#include <QObject>
#include <QVariant>
#include <QJsonValue>

#define POST_TYPE_NONE_STR			"None"
#define POST_TYPE_URL_QUERY_STR		"Url Query"
#define POST_TYPE_BODY_STR			"Body"
#define POST_TYPE_HEADER_STR		"Header"
#define POST_TYPE_UNKNOWN_STR		"Unknown"

#define POST_VALUE_TYPE_VALUE_STR				"value"
#define POST_VALUE_TYPE_UNIX_TIMESTAMP_STR		"unix_timestamp"
#define POST_VALUE_TYPE_UCENTER_AUTHCODE_STR	"ucenter_authcode"
#define POST_VALUE_TYPE_JAVASCRIPT_STR			"javascript"

#define HTTP_DATA_FIELD_KEY				"key"
#define HTTP_DATA_FIELD_USED            "used"
#define HTTP_DATA_FIELD_POST_TYPE		"post_type"
#define HTTP_DATA_FIELD_AUTH			"auth"
#define HTTP_DATA_FIELD_VALUE_TYPE		"value_type"
#define HTTP_DATA_FIELD_VALUE			"value"

class http_data : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString key READ key WRITE set_key)
    Q_PROPERTY(bool used READ used WRITE set_used)
    Q_PROPERTY(post_type_t post_type READ post_type WRITE set_post_type)
	Q_PROPERTY(bool auth READ auth WRITE set_auth)
	Q_PROPERTY(QString value_type READ value_type WRITE set_value_type)
	Q_PROPERTY(QString value READ value WRITE set_value)

public:
	Q_INVOKABLE http_data(QObject *parent = 0);
	~http_data();

	enum post_type_t{
		POST_TYPE_NONE		= 0x01,
		POST_TYPE_URL_QUERY	= 0x02,
		POST_TYPE_BODY		= 0x04,
		POST_TYPE_HEADER	= 0x08,
		POST_TYPE_ALL		= POST_TYPE_NONE|POST_TYPE_URL_QUERY | POST_TYPE_BODY | POST_TYPE_HEADER
	};

	Q_ENUM(post_type_t);

	enum auth_type_t{
		HTTP_DATA_AUTH_ALL,
		HTTP_DATA_AUTH,
		HTTP_DATA_NOT_AUTH
	};

	QString key() const;
	void set_key(const QString& key);

    bool used() const;
    void set_used(const bool used);

	post_type_t post_type() const;
	QString post_type_string() const;
	void set_post_type(post_type_t post_type);

	bool auth() const;
	void set_auth(bool auth);

	QString value_type() const;
	void set_value_type(const QString& value_type);

	QString value() const;
	void set_value(const QString value);

	QVariant data() const;
	void set_data(const QVariant& value);

public:
	static post_type_t get_post_type(const QString& post_type);
	static QString get_post_type_string(post_type_t post_type);

private:
	QString _key;
    bool _used;
	post_type_t _post_type;
	bool _auth;
	QString _value_type;
	QString _value;
	QVariant _data;
};

typedef QSharedPointer<http_data> http_data_ptr;
typedef QSharedPointer<const http_data> http_data_constptr;

class http_data_list: 
	public QList<http_data_ptr>
{
public:
	QString to_url_encode_string() const;
	QString to_url_encode_string(int post_type,
		int auth_type = http_data::HTTP_DATA_AUTH_ALL) const;
	http_data_list get_post_datas(int post_type = http_data::POST_TYPE_ALL,
        int auth_type = http_data::HTTP_DATA_AUTH_ALL, bool only_used = true) const;
	QJsonValue to_json();
	void append_from_json(const QJsonValue& v);
public:
	static http_data_list from_json(const QJsonValue& v);
private:
	static QByteArray url_encode(const QString &data);
};


#define HTTP_FORM_DATA_FIELD_NAME			"name"
#define HTTP_FORM_DATA_FIELD_USED           "used"
#define HTTP_FORM_DATA_FIELD_CONTENT_TYPE	"content_type"
#define HTTP_FORM_DATA_FIELD_VALUE			"value"

class http_form_data : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE set_name)
	Q_PROPERTY(QString content_type READ content_type WRITE set_content_type)
	Q_PROPERTY(QString value READ value WRITE set_value)
	Q_PROPERTY(QVariant data READ data WRITE set_data)

public:
	Q_INVOKABLE http_form_data(QObject *parent = 0);
	~http_form_data();

	QString name() const;
	void set_name(const QString& name);

    bool used() const;
    void set_used(const bool used);

	QString content_type() const;
	void set_content_type(const QString& content_type);

	QString value() const;
	void set_value(const QString value);

	QVariant data() const;
	void set_data(const QVariant& data);

private:
	QString _name;
    bool _used;
	QString _content_type;
	QString _value;
	QVariant _data;
};


typedef QSharedPointer<http_form_data> http_form_data_ptr;
typedef QSharedPointer<const http_form_data> http_form_data_constptr;

class http_form_data_list :
	public QList<http_form_data_ptr>
{
public:
	QJsonValue to_json();
	void append_from_json(const QJsonValue& v);
public:
	static http_form_data_list from_json(const QJsonValue& v);
};


#endif // POST_DATA_H
