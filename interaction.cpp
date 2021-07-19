#include <QtGui>
#include <QtCore>
#include <QClipboard>


extern "C"
{
// DECLARATIONS SPLIT TO AVOID CONFLICT OF TYPES

    uint32_t *getrplstackobject(int level, int *size);
    char *getdecompiledrplobject(int level, int *strsize);
    void pushobject(char *data, int sizebytes);
    void pushtext(char *data, int sizebytes);
    void removestackobject(int level, int nitems);
    int compileobject();
    void fullscreenupdate();
    void palette2list(uint32_t *buffer,int *size);
    void list2palette(uint32_t *buffer);
    int getrplobjsize(uint32_t *object);
    int palettesize();
}

// COPY OBJECT FROM THE STACK INTO THE HOST CLIPBOARD
void Stack2Clipboard(int level, int dropit)
{

    uint32_t *obj;
    int size;

    obj = getrplstackobject(level, &size);

    if(!obj)
        return;

    QClipboard *clip = qApp->clipboard();
    QMimeData *data = new QMimeData;
    QByteArray buffer;
    buffer.append((const char *)obj, size * sizeof(int32_t));

    data->setData(QString("application/newrpl-object"), buffer);

    char *text;
    int strsize;

    text = getdecompiledrplobject(level, &strsize);

    if(!text)
        return;

    QByteArray decomptext((const char *)text, strsize);
    QString utf8string(decomptext);
    data->setText(utf8string);  // PLAIN TEXT
    //data->setData(QString("text/plain;charset=utf-8"),decomptext); // UTF-8 ENCODED TEXT
    //data->setHtml(utf8string);
    clip->setMimeData(data, QClipboard::Clipboard);

    if(dropit)
        removestackobject(level, 1);

}

// PUSH THE CLIPBOARD IN THE STACK
void Clipboard2Stack()
{
    QClipboard *clip = qApp->clipboard();
    const QMimeData *data = clip->mimeData();

    // DEBUG ONLY
    for(QString& formatName: data->formats()) {
            std::string s;
            s = formatName.toStdString();

            QByteArray arr = clip->mimeData()->data(formatName);
            qDebug() << "name=" << s.c_str() << ", size=" << arr.size();
            qDebug() << "\nContents:\n";
            qDebug() << QString::fromUtf8(arr);



            qDebug() << "\n";
        }




    if(data->hasFormat(QString("application/newrpl-object"))) {
        QByteArray mydata(data->data(QString("application/newrpl-object")));

        pushobject(mydata.data(), mydata.size());
        return;
    }

    if(data->hasText()) {
        QByteArray mydata(data->text().toUtf8());
        pushtext(mydata.data(), mydata.size());
        return;
    }
}

void Clipboard2StackCompile()
{
    QClipboard *clip = qApp->clipboard();
    const QMimeData *data = clip->mimeData();

    if(data->hasFormat(QString("application/newrpl-object"))) {
        QByteArray mydata(data->data(QString("application/newrpl-object")));

        pushobject(mydata.data(), mydata.size());
        compileobject();
        return;
    }

    if(data->hasText()) {
        QByteArray mydata(data->text().toUtf8());
        pushtext(mydata.data(), mydata.size());
        compileobject();
        return;
    }
}

int SaveRPLObject(QString & filename, int level)
{
    uint32_t *obj;
    int size;

    obj = getrplstackobject(level, &size);

    if(!obj)
        return 0;

    QFile f(filename);

    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return 0;
    }

    const char *fileprolog = "NRPL";
    f.write(fileprolog, 4);
    f.write((const char *)obj, size * 4);
    f.close();

    return 1;

}

int SaveRPLObjectbyPointer(QString & filename, uint32_t * obj)
{
    int size;


    if(!obj)
        return 0;

    size=getrplobjsize(obj);

    QFile f(filename);

    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return 0;
    }

    const char *fileprolog = "NRPL";
    f.write(fileprolog, 4);
    f.write((const char *)obj, size * 4);
    f.close();

    return 1;

}



int LoadRPLObject(QString & filename)
{

    int size;

    QFile f(filename);

    if(!f.open(QIODevice::ReadOnly)) {
        return 0;
    }

    size = f.size();
    if(size & 3)
        return 0;
    QByteArray data = f.read(f.size());

    if((data.at(0) != 'N') || (data.at(1) != 'R') || (data.at(2) != 'P')
            || (data.at(3) != 'L'))
        return 0;

    pushobject(data.data() + 4, data.size() - 4);

    f.close();

    return 1;

}

void FullScreenUpdate()
{
    fullscreenupdate();
}

int SaveColorTheme(QString & filename)
{
    int palette_size = palettesize();
    uint32_t buffer[palette_size*3+2];
    int size=palette_size*3+2;
    palette2list(buffer,&size);
    if(size) return SaveRPLObjectbyPointer(filename,buffer);
    return 0;
}

int LoadColorTheme(QString & filename)
{
        int size;

        QFile f(filename);

        if(!f.open(QIODevice::ReadOnly)) {
            return 0;
        }

        size = f.size();
        if(size & 3)
            return 0;
        QByteArray data = f.read(f.size());

        if((data.at(0) != 'N') || (data.at(1) != 'R') || (data.at(2) != 'P')
                || (data.at(3) != 'L'))
            return 0;

        list2palette(((uint32_t *)data.constData())+1);

        f.close();

        FullScreenUpdate();
        return 1;
}
