## 事件编写规范

- 内部头文件统一使用 `.i.h` 后缀，禁止外部头文件引用内部头文件。
- 开源事件代码统一使用 `.open.cpp` 后缀。
- 事件编写时必须测试客户端（收包端）和服务端（发包端）是否可用；若不可用，需在事件类或函数上添加注释，例如 [WeatherUpdateEvent](src\common\ila\event\minecraft\world\WeatherUpdateEvent.h#L36) 事件。
- 若事件可拆分为“事件前”和“事件后”，则应编写一个事件基类，并让子类继承该基类；事件参数除非特殊情况，一律置于基类中。
- 事件类的成员函数，除非声明为虚函数，否则一律在头文件中实现。
- 事件类定义顺序统一为：结构体定义、成员变量、构造函数、序列化与反序列化、事件函数。
- 若事件类继承自父类且未添加其他事件参数，应使用 `using` 声明继承父类的构造函数。
- 如果事件类的函数过长、过于复杂，或客户端与服务端逻辑不同等特殊情况，可将其改为虚函数，并将实现放在对应的 `.cpp` 文件中，例如 [PacketEvent](src\common\ila\event\minecraft\packet\PacketEvent.h#L44) 事件。
- 事件参数的成员变量必须使用 `private` 或 `protected` 访问权限，并为其提供获取函数，函数名与成员变量名相同（不加 `m` 前缀）。
- 事件的序列化（`serialize`）和反序列化（`deserialize`）方法统一写在事件基类中；若事件不分前后，可忽略此项。
- 序列化时使用的键名必须采用蛇形命名法，例如 `network_system`。
- 若序列化/反序列化逻辑复杂，可优先使用 LL 的反射功能，例如 [ServerPongEvent](src\common\ila\event\minecraft\server\ServerPongEvent.open.cpp#L34) 事件。
- 若遇到未加白名单、需要使用 RVA 偏移或特征码等情况，**必须**使用 [Global.i.h](src\common\ila\base\Global.i.h) 头文件中封装的函数。
- 成员变量命名均采用 `m` + 大驼峰式，例如 `mNetworkSystem`。
- 若事件在客户端（收包端）和服务端（发包端）使用的 Hook 有差异，应将不同的 Hook 代码分别放在对应的 `client` 和 `server` 目录下。
- 若存在相同的 Hook 代码，可将共用部分放在 `common` 目录下，并创建 `事件名.i.h` 头文件，使用 `HookAliasDef` 宏定义注册 Hook 别名，同时在 Hook 代码下方使用 `HookAliasImpl` 宏定义注册别名实现，如 [WeatherUpdateEvent.cpp](src\common\ila\event\minecraft\world\WeatherUpdateEvent.cpp#L98) 跟 [WeatherUpdateEvent.i.h](src\common\ila\event\minecraft\world\WeatherUpdateEvent.i.h#L10) 。
- 事件序列化和反序列化中一律直接使用成员变量，不使用获取函数
- 重写的函数一律使用Low优先级