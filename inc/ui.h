#pragma pack(push,4)

STRUCT(item_t)
{
    STRING(type,32);
    STRING(name,64);
    STRING(value,256);
    FIELD(int,state);
    FIELD(int,user_data);
}
