#include "foo.h"

#if LIB_VERSION == 2
#include "s_v2.h"
#else
#include "s_v1.h"
#endif



struct S2
{
    const char* msg = "S2 from lib";
    const char* error = "ERROR";
};

struct S3 : S1
{
    const char* msg = "S3 from lib";
    const char* error = "ERROR";
};

namespace {

    struct S4
    {
        const char* msg = "S4 from lib";
    };

} // namespace

namespace foo {

    struct S5 : S1
    {
        const char* msg = "S5 from lib";
    };

} // namespace foo

using namespace foo;

static void throw_S1()
{
    printf("lib throw S1");
    throw S1{"S1 from lib"};
}

static void throw_S2()
{
    printf("lib throw S2");
    throw S2{};
}

static void throw_S3()
{
    printf("lib throw S3");
    throw S3{};
}

static void throw_S4()
{
    printf("lib throw S4");
    throw S4{};
}

static void throw_S5()
{
    printf("lib throw S5");
    throw S5{};
}

static std::map<std::string, type_info_t> _lib_types = {
    TYPE_MAP_ENTRY(S1),
    TYPE_MAP_ENTRY(S2),
    TYPE_MAP_ENTRY(S3),
    TYPE_MAP_ENTRY(S4),
    TYPE_MAP_ENTRY(S5)
};

template<typename E>
inline void lib_handle_exception(const E& e)
{
    printf(" -> lib caught: %s msg=\"%s\"\n", typeid(decltype(e)).name(), e.msg);
}

void lib_try_catch(f_t f)
{
    try {
        f();
    } catch (const S5& e) {
        lib_handle_exception(e);
    } catch (const S4& e) {
        lib_handle_exception(e);
    } catch (const S3& e) {
        lib_handle_exception(e);
    } catch (const S2& e) {
        lib_handle_exception(e);
    } catch (const S1& e) {
        lib_handle_exception(e);
    } catch (...) {
        printf(" -> lib caught: unknown exception\n");
    }
}

const std::map<std::string, type_info_t>& get_lib_types()
{
    return _lib_types;
}
