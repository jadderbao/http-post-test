#ifndef POST_DATA_PROCESS_H
#define POST_DATA_PROCESS_H

#include <QObject>
#include "http_data.h"

class http_script_engine;
class http_data_list; 

class post_data_process : public QObject
{
	Q_OBJECT

public:
	post_data_process(http_script_engine *engine, QObject *parent);
	~post_data_process();
	bool process_post_data(http_data_list *datas);
	QString error_string();
private:
	http_script_engine *_engine;
	QString _error_string;
};

#endif // POST_DATA_PROCESS_H
