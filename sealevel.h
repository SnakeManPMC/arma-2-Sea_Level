#ifndef SEALEVEL_H
#define SEALEVEL_H

typedef struct
{
    short Elevation;
    short TexIndex;
} wrpformat[4096][4096];

class seaLevel
{
public:
    seaLevel(int argc, char *argv[]);
    void Open_Files(int argc, char *argv[]);
    void Read_Signature();
    void Read_Elevations();
    void Read_Textures();
    void Write_Elevations();
    void Write_Textures();
    void Read_Objects();
    void Close_Files();

private:
    FILE *map;
    FILE *output;
    char sig[33];
    short TexIndex;
    int MapSize, SeaLevelAdjust;

};

#endif // SEALEVEL_H
