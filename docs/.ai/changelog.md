# 更新日志

本文件记录 StickersManager 的所有重要变更。**新条目加在顶部**。

---

## [2025-01-14]

### 初始记录

- 建立 docs/.ai 结构（Init 协议）
  - 扫描来源：README.md、AGENTS.md、CMakeLists.txt、src/main.cpp、src/appinfo.h、src/core/configmanager.h/.cpp、src/ui/mainwindow.h、.github/workflows/release.yml
  - 已建：index.md（红线+分层路由）、core/（project-overview / architecture / quick-start）、modules/（core-config / core-library / core-image / core-input / core-recent / ui-mainwindow / ui-settings / ui-tray）、config/（config-json / settings-json）、best-practices/（development / deployment）
  - 待验证项：
    - `default.window.position/size` 具体数值（configmanager.cpp:14-15，hardDefaults 实测）
    - settings.json 默认值（configmanager.cpp:58-66，defaultSettings 实测）
    - 版本 2.7.1 与 CHANGELOG.md 顶部是否同步（appinfo.h:7 实测当前值）

<!--
格式：`- 功能/修复/重构标题` + 缩进子项（改了什么 + 为什么 + 关键细节）
示例：
- 导航栏自动隐藏
  - 桌面端 hover 顶部 100px 下拉显示
  - 移动端始终显示（userAgent 检测，非屏幕宽度）
  - 环境变量 NEXT_PUBLIC_HEADER_AUTO_HIDE_ENABLED 仅作用桌面端
-->
