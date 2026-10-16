# أدوات البناء

- `BuildAndroid.ps1` يبني هدف `VelocityRushEditor` ثم يشغّل UAT لتغليف APK. يحتاج Windows وUnreal 5.5.x وAndroid SDK/NDK/JDK مهيأة في المحرك. عيّن `UE_ROOT` قبل التشغيل.
- `.github/workflows/build-unreal-android.yml` يعيد استخدام نفس المتطلبات على Runner ذاتي الاستضافة بالوسم `unreal-android`، ويتحقق من وجود APK قبل رفعه.

في بيئة لا تحتوي Unreal Engine، لا يمكن لهذا السكربت أن يحل محل المحرك أو أن يثبت نجاح البناء.
