/**
* @file    winmodbase.h
* @brief   ...
* @author  bbcallen
* @date    2009-02-25  11:12
*/

#ifndef WINMODBASE_H
#define WINMODBASE_H

#ifndef NS_WINMOD_BEGIN
#define NS_WINMOD_BEGIN     namespace WinMod {
#define NS_WINMOD_END       };
#define NS_WINMOD_USING     using namespace WinMod;
#endif//NS_WINMOD_BEGIN

NS_WINMOD_BEGIN

NS_WINMOD_END


/// 构造winmod模块的http成功码
#define MAKE_WINMOD_HTTP_SUCCESS(x)             HRESULT(0x60020000 | (0xFFFF & x))
/// 构造winmod模块的http错误码
#define MAKE_WINMOD_HTTP_ERROR(x)               HRESULT(0xA0020000 | (0xFFFF & x))


#define WINMOD_HTTP__APPLICATION__X_WWW_FORM_URLENCODED     L"application/x-www-form-urlencoded"
#define WINMOD_HTTP__APPLICATION__X_OCTET_STREAM            L"application/octet-stream"



class CWinModBits
{
public:
    template <class L, class R>
    static inline void DoSet(L& value, R bits)
    {
        value |= bits;
    }

    template <class L, class R>
    static inline void UnSet(L& value, R bits)
    {
        value &= ~bits;
    }

    template <class L, class R>
    static inline bool MatchAll(L& value, R bits)
    {
        return bits == (value & bits);
    }

    template <class L, class R>
    static inline bool MatchAny(L& value, R bits)
    {
        return 0 != (value & bits);
    }
};



#endif//WINMODBASE_H