🍎 Console-Snake-Plus

基于 C 语言控制台的贪吃蛇游戏，代码量约 2000 行。在经典玩法上加入了毒苹果、复活甲、双模式障碍物、多档位音效以及积分排行榜，主界面由蛇吃苹果来选择功能。

游戏特色

苹果菜单导航

· 主界面显示多个选项，每个选项前有一颗苹果。 · 玩家使用 W/A/S/D 控制蛇移动，吃掉对应的苹果即可进入“开始游戏、音效设置、排行榜、游戏说明、退出游戏”等功能。

普通模式 & 困难模式

· 普通模式：经典贪吃蛇，速度适中。 · 困难模式：蛇移动速度加快，地图中会随机生成障碍物，挑战性更高。

毒苹果与复活甲

· 游戏中有概率出现毒苹果。 · 95% 概率：吃掉后线性扣除积分。 · 5% 概率：生成复活甲。 · 每局开始玩家自动携带 1 个复活甲（上限 1 个）。 · 蛇死亡时若持有复活甲，会自动消耗并原地复活一次。 · 如果已经拥有复活甲，再触发 5% 概率事件会直接加积分。

音效系统

· 主菜单的每一个选项、普通模式、困难模式都有独立的背景音乐。 · 蛇死亡、吃到毒苹果等事件有专属音效。 · 在“音效设置”中可以打开/关闭音效，并调节音量（1~6 档）。

排行榜

· 游戏结束后记录昵称、序号和积分。 · 显示历史积分前 10 名，同昵称只保留最高分。

其他

· 游戏说明中可查看完整规则。 · 退出游戏选项会直接关闭程序。

操作方式

按键 功能 W 向上移动 S 向下移动 A 向左移动 D 向右移动 空格/回车 确认选择 ESC 返回上级菜单/退出

菜单界面通过控制蛇去吃苹果来选择功能。

English Translation

🍎 Console-Snake-Plus

A console-based Snake game written in C, with about 2000 lines of code. It expands on the classic gameplay by adding poison apples, revive armor, dual modes with obstacles, multi-level sound effects, and a points leaderboard. The main menu is navigated by guiding a snake to eat apples that correspond to different options.

Game Features

Apple Menu Navigation

· The main screen displays several options, each marked with an apple.
· Players control the snake with W/A/S/D. Eating the apple next to an option enters that function: Start Game, Sound Settings, Leaderboard, Game Instructions, Exit Game.

Normal Mode & Hard Mode

· Normal Mode: Classic Snake game with moderate speed.
· Hard Mode: Increased snake speed, and obstacles randomly appear on the map for a greater challenge.

Poison Apple & Revive Armor

· Poison apples may appear randomly during the game.
  · 95% chance: Eating one deducts points linearly.
  · 5% chance: A revive armor is generated.
· At the start of each game, the player automatically carries 1 revive armor (max 1).
· When the snake dies while holding a revive armor, it is consumed and the snake revives on the spot.
· If the player already has a revive armor, the 5% event grants bonus points instead.

Sound System

· Every main menu option, Normal Mode, and Hard Mode has its own background music (BGM).
· Special sound effects are triggered on death and when eating a poison apple.
· In Sound Settings, you can toggle sound on/off and adjust the volume (levels 1–6).

Leaderboard

· After each game, the player's nickname, rank number, and score are recorded.
· The top 10 highest scores are displayed; only the highest score per nickname is kept.

Other

· Full game rules can be viewed in Game Instructions.
· Selecting Exit Game closes the program.

Controls

Key Action
W Move Up
S Move Down
A Move Left
D Move Right
Space/Enter Confirm Selection
ESC Return to Previous Menu / Exit

