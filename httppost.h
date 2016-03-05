#ifndef HTTPPOST_H
#define HTTPPOST_H

#include <QtWidgets/QMainWindow>
#include "ui_httppost.h"
#include "config.h"

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QNetworkCookieJar;
class QProgressBar;
class QHttpMultiPart;

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
	QByteArray url_encode(QString &key);
	QJsonObject key_value_table_to_json(QTableWidget *tableWidget);
	void key_value_json_to_table(QJsonObject &v, QTableWidget *tableWidget);
	void form_data_table_to_multi_part(QHttpMultiPart* multi_part, QTableWidget *tableWidget);
	void update_request_custom_header(QNetworkRequest *request);
	void insert_form_data(const QString& type, const QString& file_name);

public slots:
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void finished();

	void post();
	void get();

	void insert();
	void remove();
	void save();
	void load();

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

private:
	Ui::httppostClass ui;
	QNetworkAccessManager *am;
	QNetworkCookieJar *cookie_jar;
	QProgressBar *progress_bar;
	QString last_file_name;
};

#endif // HTTPPOST_H
