#include <Engine/exception.hpp>
#include <src/log.hpp>
#include <Engine/data_source/stock_data.hpp>

#include <lua/lua.hpp>

namespace prescyengine {

static double lua_result = 0;
static int lua_evaluate(lua_State* L) {
    lua_result = lua_tonumber(L, 1);
    return 0;
}

double Prescy_Engine evaluateExpression(const std::vector<StockData>& data, const std::string& expression) {
    auto luaState_deleter = [](lua_State* L) {
        lua_close(L);
    };
    std::unique_ptr<lua_State, std::function<void(lua_State*)>> luaState(luaL_newstate(), luaState_deleter);
    lua_State* L = luaState.get();
    luaL_openlibs(L);
    lua_pushcfunction(L, lua_evaluate);
    lua_setglobal(L, "setResult");

    lua_settop(L, 0);
    lua_createtable(L, 0, static_cast<int>(data.size()));
    for (std::size_t i = 0; i < data.size(); ++i) {
        lua_createtable(L, 0, 6);
        lua_pushinteger(L, data[i].timeStamp);
        lua_setfield(L, -2, "timeStamp");
        lua_pushnumber(L, data[i].open);
        lua_setfield(L, -2, "open");
        lua_pushnumber(L, data[i].close);
        lua_setfield(L, -2, "close");
        lua_pushnumber(L, data[i].high);
        lua_setfield(L, -2, "high");
        lua_pushnumber(L, data[i].low);
        lua_setfield(L, -2, "low");
        lua_pushnumber(L, data[i].volume);
        lua_setfield(L, -2, "volume");
        lua_rawseti(L, -2, static_cast<int>(i));
    }
    lua_pushinteger(L, static_cast<int>(data.size()));
    lua_setglobal(L, "__size");
    lua_setglobal(L, "data");

    lua_result = 0;
    if (luaL_dostring(L, expression.c_str()) != 0) {
        auto luaError = lua_tostring(L, -1);
        throw PrescyException(fmt::format("Error evaluating expression: Lua Error: {}", luaError));
    }
    return lua_result;
}

}
