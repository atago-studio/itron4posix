# itron4posix
POSIXを使用したITRON4環境です。

ITRONに慣れたプログラマ（私です）が初めてLinuxでThreadプログラミングした際に感じるのはOSプリミティブ（API）の少なさではないでしょうか。
POSIXに対して使い慣れたITRON APIでアクセスできたら開発が楽になると思いました。

本プロジェクトはPOSIXに対するITRONラッパとなります。
TOPPERSのように完璧なITRON OSの動作をシミュレートする＊＊ものではありません＊＊

POSIXやLinuxのタスクスイッチング機構に依存していますので、タスク優先度に依存した処理
（優先度の高いタスクが優先度の低いタスクをブロッキングする事を前提とした処理）
は正常に動作しない可能性があります。

一方で、完璧なITRONシミュレーションのようなオーバーヘッドが殆どありませんのでサイズ、動作とも軽いと思います。

個人のプロジェクトとして制作しましたが、多くの組込プログラマのお役に立てればと思い公開します。


あたご工房
Atago Engineering studio.
