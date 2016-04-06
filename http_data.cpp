#include "http_data.h"
#include <QSharedPointer>
#include <QJsonArray>
#include <QJsonObject>

//post_data

Q_INVOKABLE http_data::http_data(QObject *parent /*= 0*/)
	:QObject(parent)
{

}

http_data::~http_data()
{

}

QString http_data::key() const
{
	return _key;
}

void http_data::set_key(const QString& key)
{
	_key = key;
}

http_data::post_type_t http_data::post_type() const
{
	return _post_type;
}

QString http_data::post_type_string() const
{
	return get_post_type_string(post_type());
}

void http_data::set_post_type(http_data::post_type_t post_type)
{
	_post_type = post_type;
}

bool http_data::auth() const
{
	return _auth;
}

void http_data::set_auth(bool auth)
{
	_auth = auth;
}

QString http_data::value_type() const
{
	return _value_type;
}

void http_data::set_value_type(const QString& value_type)
{
	_value_type = value_type;
}

QString http_data::value() const
{
	return _value;
}

void http_data::set_value(const QString value)
{
	_value = value;
}

QVariant http_data::data() const
{
	return _data;
}

void http_data::set_data(const QVariant& value)
{
	_data = value;
}


const QMap<QString, http_data::post_type_t> & get_post_type_map(){
	static QMap<QString, http_data::post_type_t> post_types;
	
	if(post_types.empty()){
		post_types.insert(POST_TYPE_NONE_STR, http_data::POST_TYPE_NONE);
		post_types.insert(POST_TYPE_URL_QUERY_STR, http_data::POST_TYPE_URL_QUERY);
		post_types.insert(POST_TYPE_BODY_STR, http_data::POST_TYPE_BODY);
		post_types.insert(POST_TYPE_HEADER_STR, http_data::POST_TYPE_HEADER);
	};

	return post_types;
}

http_data::post_type_t http_data::get_post_type(const QString& post_type)
{
	return get_post_type_map()[post_type];
}

QString http_data::get_post_type_string(post_type_t post_type)
{
	const QMap<QString, http_data::post_type_t>& map = get_post_type_map();
	for (auto it = map.begin(); it != map.end(); ++it){
		if (*it == post_type){
			return it.key();
		}
	}

	return QString("Unknown");
}

//post_date_list
QByteArray http_data_list::url_encode(const QString &data)
{
	QByteArray ba = data.toUtf8();
	return ba.toPercentEncoding();
}


QString http_data_list::to_url_encode_string() const
{
	QString str;
	for (auto it = begin(); it != end(); it++){
		http_data_ptr pdi = *it;
		str.append(url_encode(pdi->key()));
		str.append("=");

		str.append(url_encode(pdi->value()));
		str.append("&");
	}

	if (!str.isEmpty()){
		str.chop(1);
	}

	return str;
}

QString http_data_list::to_url_encode_string(int post_type, 
	int auth_type ) const
{
	http_data_list temp_items = get_post_datas(post_type, auth_type);
	return temp_items.to_url_encode_string();
}

http_data_list http_data_list::get_post_datas(int post_type, int auth_type) const
{
	http_data_list temp_items;
	for (auto it = begin(); it != end(); it++){
		http_data_ptr pdi = *it;
		if (pdi->post_type() & post_type
			&& (auth_type == http_data::HTTP_DATA_AUTH_ALL
			|| auth_type == http_data::HTTP_DATA_AUTH && pdi->auth()
			|| auth_type == http_data::HTTP_DATA_NOT_AUTH && !pdi->auth())){

			temp_items.append(pdi);
		}
	}

	return temp_items;
}

QJsonValue http_data_list::to_json()
{
	QJsonArray arr;
	for (int i = 0; i < size(); i++){
		const http_data_ptr& data_ptr = at(i);
		QJsonObject obj;
		obj[HTTP_DATA_FIELD_KEY] = data_ptr->key();
		obj[HTTP_DATA_FIELD_POST_TYPE] = data_ptr->post_type();
		obj[HTTP_DATA_FIELD_AUTH] = data_ptr->auth();
		obj[HTTP_DATA_FIELD_VALUE_TYPE] = data_ptr->value_type();
		obj[HTTP_DATA_FIELD_VALUE] = data_ptr->value();
		arr.append(obj);
	}

	return arr;
}

void http_data_list::append_from_json(const QJsonValue& v)
{
	append(from_json(v));
}

http_data_list http_data_list::from_json(const QJsonValue& v)
{
	if (!v.isArray()){
		return http_data_list();
	}

	http_data_list list;

	QJsonArray arr = v.toArray();
	for (int i = 0; i < arr.size(); i++){
		http_data_ptr data_ptr(http_data_ptr::create());
		QJsonObject obj = arr[i].toObject();
		data_ptr->set_key(obj[HTTP_DATA_FIELD_KEY].toString());
		data_ptr->set_post_type((http_data::post_type_t)obj[HTTP_DATA_FIELD_POST_TYPE].toInt());
		data_ptr->set_auth(obj[HTTP_DATA_FIELD_AUTH].toBool());
		data_ptr->set_value_type(obj[HTTP_DATA_FIELD_VALUE_TYPE].toString());
		data_ptr->set_value(obj[HTTP_DATA_FIELD_VALUE].toString());

		list.append(data_ptr);
	}

	return list;
}

//http_form_data
Q_INVOKABLE http_form_data::http_form_data(QObject *parent /*= 0*/)
	:QObject(parent)
{

}

http_form_data::~http_form_data()
{

}

QString http_form_data::name() const
{
	return _name;
}

void http_form_data::set_name(const QString& name)
{
	_name = name;
}

QString http_form_data::content_type() const
{
	return _content_type;
}

void http_form_data::set_content_type(const QString& content_type)
{
	_content_type = content_type;
}

QString http_form_data::value() const
{
	return _value;
}

void http_form_data::set_value(const QString value)
{
	_value = value;
}

QVariant http_form_data::data() const
{
	return _data;
}

void http_form_data::set_data(const QVariant& data)
{
	_data = data;
}


//http_form_data_list
QJsonValue http_form_data_list::to_json()
{
	QJsonArray arr;
	for (int i = 0; i < size(); i++){
		const http_form_data_ptr& data_ptr = at(i);
		QJsonObject obj;
		obj[HTTP_FORM_DATA_FIELD_NAME] = data_ptr->name();
		obj[HTTP_FORM_DATA_FIELD_CONTENT_TYPE] = data_ptr->content_type();
		obj[HTTP_DATA_FIELD_VALUE] = data_ptr->value();
		arr.append(obj);
	}

	return arr;
}

void http_form_data_list::append_from_json(const QJsonValue& v)
{
	append(from_json(v));
}

http_form_data_list http_form_data_list::from_json(const QJsonValue& v)
{
	if (!v.isArray()){
		return http_form_data_list();
	}

	http_form_data_list list;

	QJsonArray arr = v.toArray();
	for (int i = 0; i < arr.size(); i++){
		http_form_data_ptr data_ptr(http_form_data_ptr::create());
		QJsonObject obj = arr[i].toObject();
		data_ptr->set_name(obj[HTTP_FORM_DATA_FIELD_NAME].toString());
		data_ptr->set_content_type(obj[HTTP_FORM_DATA_FIELD_CONTENT_TYPE].toString());
		data_ptr->set_value(obj[HTTP_DATA_FIELD_VALUE].toString());

		list.append(data_ptr);
	}

	return list;
}
