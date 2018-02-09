#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QHttpMultiPart;
class QIODevice;

class http_request_body
{
public:
	enum request_body_type_t{
		REQUEST_BODY_TYPE_NONE,
		REQUEST_BODY_TYPE_DATA,
		REQUEST_BODY_TYPE_MULTI_PART,
        REQUEST_BODY_TYPE_IO_DEVICE
	};

	union request_body_data_t{
		request_body_data_t(QByteArray *d)
            :data(d)
		{}

		request_body_data_t(QHttpMultiPart *m)
            :multi_part(m)
		{}

        request_body_data_t(QIODevice *io_device)
            :io_device(io_device)
        {}
		QByteArray *data;
		QHttpMultiPart *multi_part;
        QIODevice *io_device;
	};

	http_request_body(const QByteArray& data);
	http_request_body(QHttpMultiPart *multi_part);
    http_request_body(QIODevice *io_device);
    ~http_request_body();
	request_body_type_t type();
	const QByteArray data() const;
	QHttpMultiPart *multi_part();
    QIODevice *io_device();
private:
	request_body_type_t _body_type;
	request_body_data_t _body_data;
};

class http_request : public QObject
{
	Q_OBJECT

public:
	http_request(const QUrl& url, QObject *parent = 0);
	~http_request();
	QNetworkRequest *request() const;
	void set_url(const QUrl& url);
	void set_body(http_request_body *body);
	http_request_body *body() const;
private:
	QNetworkRequest *_request;
	http_request_body *_body;
};


class http_client : public QObject
{
	Q_OBJECT

Q_SIGNALS :
	void finished();
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

public:
	http_client(QNetworkAccessManager *am, QObject *parent = 0);
	~http_client();
	QNetworkReply *post(http_request *request, bool redirect = true);
	QNetworkReply *get(http_request *request, bool redirect = true);
    QNetworkReply *customRequest(http_request *request, QString& verb, bool redirect = true);
	QByteArray get_reply_data(QNetworkReply *reply);
    bool process_and_check_redirect(bool redirect, QNetworkReply *reply, QNetworkRequest& req);
        
private:
	QNetworkAccessManager *_am;
};

#endif // HTTP_CLIENT_H
