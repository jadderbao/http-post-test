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
class QWebEngineView;

class http_script_engine;
class QUrl;
class http_request;
class QJsonObject;
class QMenu;
class QTime;

typedef QMap<QString, QString> key_value_map;

class httppost : public QMainWindow
{
	Q_OBJECT

public:
	httppost(QWidget *parent = 0);
    ~httppost();
    void add_save_button_menu();
    void show_reply(QNetworkReply * reply, QTime *time);
	QString get_url_encode_body();

    void set_ui_data(QJsonObject& v);
    QJsonObject get_ui_data();
	QByteArray url_encode(const QString &key);
	QJsonValue form_data_table_to_json(QTableWidget *tableWidget);
	void form_data_json_to_table(QJsonValue &v, QTableWidget *tableWidget);
	QJsonValue headers_table_to_json(QTableWidget *tableWidget);
	void headers_json_to_table(QJsonValue &v, QTableWidget *tableWidget);
	void form_data_table_to_multi_part(QHttpMultiPart* multi_part, QTableWidget *tableWidget);
	void update_body_to_multi_part(const http_data_list &items, QHttpMultiPart * multi_part);
	void insert_form_data(const QString& type, const QString& file_name);

protected:
    void set_table_widgets(const QJsonObject& form_status_data);

protected:
	bool get_ui_data_items(http_data_list& items);
	void add_post_data_item(http_data_list& items, http_data_ptr item);
	bool process_and_update_item_value(http_data_list& items, QString& error);

public:
	void update_request_custom_header(const http_data_list& items, QNetworkRequest *request);
    QUrl get_url();
    bool update_request_body(http_request* request);
    QString get_combobox_items(QComboBox *comboBox);
    QStringList get_http_content_type(const QJsonObject& form_status_data);
public:
    void save_form_status();
    void load_form_status(const QJsonObject& form_status_data);

public:
	void initialize_script_engine(http_script_engine *engine);

public slots:
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void finished();

    void content_type_button_clicked(bool checked);
    void custom_type_button_clicked(bool checked);

    void custom_open_file();

    void send();

    void load();
    void save();
    void save_as();

	void form_add_text();
	void form_add_image();
	void form_add_raw();

public:
	QString get_open_file_name(const QString& filter);
	QString get_save_file_name(const QString& filter);
	QByteArray load_text_file(const QString& filter);
	void save_text_file(const QString& filter, const QByteArray& data);
    QJsonObject load_json_file(const QString& file_name, bool show_error = true);
	void save_json_file(const QString& file_name, const QJsonObject& v);

    void update_custom_type_ui(int custom_type);

private:
	Ui::httppostClass ui;
	QNetworkAccessManager *am;
	QNetworkCookieJar *cookie_jar;
	QProgressBar *progress_bar;
	QString last_file_name;
	http_script_engine *_engine;
    QMenu *_file_menu;
    QWebEngineView * _webView;
};

#endif // HTTPPOST_H
