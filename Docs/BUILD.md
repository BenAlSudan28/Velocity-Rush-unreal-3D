# البناء والتغليف

## المتطلبات

- Unreal Engine **5.5.x** مرخّص ومثبت على Windows 11/Windows Server متوافق.
- Visual Studio 2022 مع حزم C++ الخاصة بـ Unreal و.NET الذي يطلبه إصدار المحرك.
- Android SDK/NDK/JDK من الإصدارات التي يوصي بها هذا التثبيت من UE 5.5، مع قبول التراخيص واجتياز `Turnkey VerifySdk`.
- Runner ذاتي الاستضافة في GitHub Actions بوسم `unreal-android`، ومتغير Actions `UE_ROOT` يشير إلى مجلد تثبيت Unreal.

لا تستخدم `windows-latest` العام لبناء APK: GitHub لا يضم Unreal Engine. لا تنزّل المحرك من مصدر غير رسمي، ولا تحفظ بيانات Epic أو مفاتيح التوقيع في المستودع.

## فتح وتشغيل

افتح `VelocityRush.uproject` في UE 5.5.x، وافق على إنشاء ملفات المشروع الثنائية، ثم اضغط **Play**. يبدأ التشغيل بالقائمة. ابدأ سباق السرعة لمنافسة أربعة خصوم، أو اختر القيادة الحرة. للتحكم: `W` تسارع، `A/D` أو الأسهم توجيه، `S` أو `Space` فرامل، `Left Shift` نيترو، و`Esc` إيقاف مؤقت.

## إدخال النماذج والخط والصوت

المجلد `Resources/` يحوي ملفات مصدر، وليس حزم Unreal الثنائية (`.uasset`). استورد ملفات السيارات `.glb` إلى `/Game/Vehicles/` مع الحفاظ على أسماء الأصول `AerionGT` و`VortexSVX` و`TerraRallyS`. بعد الاستيراد سيستخدمها الكود عند اختيار السيارة؛ وإذا لم توجد، يبقى المجسم الإجرائي البديل. استورد ملفات WAV إلى `/Game/Audio/` وخطوط Noto إلى `/Game/Fonts/` عند إعداد دعم الصوت والخط داخل المحرر. أسماء WAV ومصدر الخط موثّقان في `Resources/README.md`.

يُدرج إعداد التغليف مجلدات `/Game/Vehicles` و`/Game/Audio` و`/Game/Fonts` في الطبخ بعد إضافة الأصول إلى المشروع. راجع حقوق ملفات الصوت قبل إعادة توزيع اللعبة تجاريًا؛ أرشيف المستخدم لم يتضمن بيان ترخيص منفصل لها.

## بناء APK محليًا

من PowerShell في مجلد المشروع، عيّن مسار تثبيت Unreal ثم نفّذ:

```powershell
$env:UE_ROOT = "C:\Program Files\Epic Games\UE_5.5"
.\Tools\BuildAndroid.ps1
```

يجمع السكربت هدف المحرر، ثم ينفّذ `BuildCookRun` بصيغة `ETC2` ويتحقق من وجود APK. إذا رفض المحرك إعداد SDK، افتح Project Settings > Platforms > Android واتبع إصدارات SDK/NDK/JDK المتوافقة التي يذكرها **هذا المحرك** ثم أعد `Turnkey VerifySdk`.

## GitHub Actions

`/.github/workflows/build-unreal-android.yml` يعمل يدويًا وعند الدفع إلى `main`. أضف Runner Windows ذاتي الاستضافة يحمل الوسم `unreal-android`، وثبّت عليه UE 5.5.x وتهيئة Android. عرّف متغير المستودع `UE_ROOT` بمسار المحرك. يتحقق Workflow من SDK، ويبني، ويطبخ، ثم يفشل إن لم يجد APK، ويرفع الملف وسجلات UAT كـ artifacts. إذا غاب المحرك أو الـ SDK يفشل برسالة واضحة بدل نجاح وهمي.

## حدود التحقق

لم يُجمّع هذا المشروع داخل Unreal في بيئة الإنشاء الحالية لعدم توفر Unreal/UnrealBuildTool. الفحص الساكن ليس بديلًا عن UBT أو تجربة APK على هاتف Android.
