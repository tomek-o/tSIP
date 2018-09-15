//---------------------------------------------------------------------------

#ifndef KeybKeysH
#define KeybKeysH
//---------------------------------------------------------------------------

struct vkey {
	char* name;
	unsigned char vkey;
	char* description;
};
extern const vkey vkey_list[];
int vkey_list_size(void);
int vkey_find(const char* name);

#endif
