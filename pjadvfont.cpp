#include <QApplication>
#include <QFont>
#include <QFile>
#include <QString>
#include <iostream>
#include <QFile>
#include <QPixmap>
#include <QTextStream>
#include <QList>
#include <QPainter>
#include <QChar>

//predefined args
int nHeight = 70;
int nWeight = 70;
int nFontsize = 45;

struct pjadv_glyph{
    char magic[12];
    int glyph_count;
    int fullsize_character_width;
    int halfsize_character_width;
    int fullsize_glyph_size;
    int halfsize_glyph_size;
};

using namespace std;

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    if(argc<4)
    {
        cout<<"Usage:"<<argv[0]<<" TBL_file glyphsize fontsize"<<endl;
        return -1;
    }

    nHeight = nWeight = atoi(argv[2]);
    nFontsize = atoi(argv[3]);
    int gcount = 0;
    QFile qf_TBL(argv[1]);

    if(!qf_TBL.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        cout<<"File can not open!"<<endl;
        return -1;
    }

    QTextStream in_str(&qf_TBL);
    in_str.setCodec("UTF-16LE");
    QList<QString> ProcChrList;
    QByteArrayList ProcCodeList;

    while(!in_str.atEnd())
    {
        auto fetch_str=in_str.readLine();
        auto Encode=fetch_str.split('=');
        ProcChrList.append(Encode[1]);
        //wtf
        QByteArray tmp = QByteArray::fromHex(Encode[0].toLocal8Bit()).toHex();
        ProcCodeList.append(tmp);
        gcount++;
    }

    cout<<"TBL read!"<<endl<<"Writing the Font file"<<endl;
    //write necessary information into struct
    pjadv_glyph glyph;
    strcpy_s(glyph.magic,12,"PF001Lillian");
    glyph.glyph_count = gcount;
    glyph.fullsize_character_width= nWeight;
    glyph.halfsize_character_width = nWeight/2;
    glyph.fullsize_glyph_size = nWeight*nWeight;
    glyph.halfsize_glyph_size =(nWeight*nWeight)/2;
    QByteArray outputBuf;
    outputBuf.append((char*)&glyph, sizeof(glyph));

    uint encodechar = 0;
    for(auto i=0;i<ProcChrList.size();i++)
    {
        nWeight = nHeight;
        encodechar = ProcCodeList[i].toUInt(nullptr, 16);
        if(encodechar<0x100){
            //half size glyph
            nWeight = nWeight/2;
        }
        auto pix=new QPixmap(nWeight,nHeight);
        auto p=new QPainter (pix);
        p->fillRect(QRect(QPoint(0,0),QPoint(nWeight,nHeight)),QBrush(QColor(Qt::black).rgb()));
        p->setFont(QFont("黑体", nFontsize));
        p->setPen(QColor(Qt::white));
        p->drawText(QRect(QPoint(0,0),QPoint(nWeight,nHeight)),Qt::AlignCenter,ProcChrList[i]);
        p->end();
        QImage img=pix->toImage();
        delete p;
        delete pix;
        auto grayImg=img.convertToFormat(QImage::Format_Grayscale8);
        outputBuf.append(encodechar>>8);
        outputBuf.append(encodechar);
        if (encodechar<0x100)outputBuf.append('\0');
        for(int y=0;y<nHeight;y++)
        {
            for(int x=0;x<nWeight;x++)
            {
                auto val=qGray(grayImg.pixel(x,y));
                outputBuf.append(val);
            }
        }
    }
    QFile outFile("output.bin");
    if(!outFile.open(QIODevice::WriteOnly))
    {
        cout<<"File can not open!"<<endl;
        return -1;
    }

    outFile.write(outputBuf);
    outFile.close();
    cout<<"Finished!"<<endl;
    return 0;
}
