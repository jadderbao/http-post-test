#ifndef HTTPPOST_H
#define HTTPPOST_H

#include <QtWidgets/QMainWindow>
#include "ui_httppost.h"
#include "config.h"
#include "http_data.h"
#include <QJsonValue>

class http_request_body;

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QNetworkCookieJar;
class QProgressBar;
class QHttpMultiPart;
class QTableWidget;

class http_script_engine;

typedef QMap<QString, QString> key_value_map;

class httppost : public QMainWindow
{
	Q_OBJECT

public:
	httppost(QWidget *parent = 0);
	~httppost();
	void show_reply(QNetworkReply * reply);
	QString get_url_encode_body();

	void load_url_encode_post_data(const QString& file_name);
	void save_url_encode_post_data(const QString& file_name);
	QByteArray url_encode(const QString &key);
	QJsonValue form_data_table_to_json(QTableWidget *tableWidget);
	void form_data_json_to_table(QJsonValue &v, QTableWidget *tableWidget);
	QJsonValue headers_table_to_json(QTableWidget *tableWidget);
	void headers_json_to_table(QJsonValue &v, QTableWidget *tableWidget);
	void form_data_table_to_multi_part(QHttpMultiPart* multi_part, QTableWidget *tableWidget);
	void update_body_to_multi_part(const http_data_list &items, QHttpMultiPart * multi_part);
	void insert_form_data(const QString& type, const QString& file_name);

protected:
	void set_table_widgets();

protected:
	bool get_ui_data_items(http_data_list& items);
	void add_post_data_item(http_data_list& items, http_data_ptr item);
	bool process_and_update_item_value(http_data_list& items, QString& error);

	void http_post(QUrl &url, http_request_body * http_body, const http_data_list &items,
		const QString& content_type = QString());
	void http_post(QUrl &url, const QByteArray &body, const http_data_list &items, const QString& content_type);
	void http_post(QUrl &url, QHttpMultiPart *multi_part, const http_data_list &items);

	void post_url_encoded(QUrl& url);
	void post_byte_data(QUrl& url, const QByteArray& data, const QString& content_type);
	void post_multi_part(QUrl& url);
public:
	void update_request_custom_header(const http_data_list& items, QNetworkRequest *request);

public:
	void initialize_script_engine(http_script_engine *engine);

public slots:
	void move_row_up();
	void move_row_down();

	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void finished();

	void post();
	void get();

	void url_encode_body_insert();
	void url_encode_body_remove();
	void url_encode_body_save();
	void url_encode_body_load();

	void form_add_text();
	void form_add_image();
	void form_add_raw();

	void form_insert();
	void form_remove();

	void form_save();
	void form_load();

	void json_save();
	void json_load();

	void text_save();
	void text_load();

	void header_insert();
	void header_remove();

	void header_save();
	void header_load();

public:
	QString get_open_file_name(const QString& filter);
	QString get_save_file_name(const QString& filter);
	QByteArray load_text_file(const QString& filter);
	void save_text_file(const QString& filter, const QByteArray& data);
	QJsonObject load_json_file(const QString& file_name);
	void save_json_file(const QString& file_name, const QJsonObject& v);

private:
	Ui::httppostClass ui;
	QNetworkAccessManager *am;
	QNetworkCookieJar *cookie_jar;
	QProgressBar *progress_bar;
	QString last_file_name;
	http_script_engine *_engine;
};

#endif // HTTPPOST_H
