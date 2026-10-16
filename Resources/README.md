# Resources

## Models

ملفات `.glb` هنا هي مصادر ثلاث سيارات أصلية خيالية بطابع عام: كوبيه إيطالية حمراء، سوبركار برتقالية، وكوبيه رياضية زرقاء. لا تحمل شعارات Ferrari أو Lamborghini أو Toyota ولا تدّعي ترخيصًا أو نسخ موديل إنتاجي.

استورد كل نموذج في Unreal Content Browser إلى `/Game/Vehicles/` مع إبقاء الاسم الأساسي:

- `AerionGT.glb` → `AerionGT`
- `VortexSVX.glb` → `VortexSVX`
- `TerraRallyS.glb` → `TerraRallyS`

سيستخدمها `AVelocityRushVehiclePawn` تلقائيًا بعد الاستيراد؛ وإلا يظهر هيكل احتياطي مبني وقت التشغيل. النماذج الخام ليست `.uasset`، ولم يُختبر استيرادها داخل المحرك في هذه البيئة. قد يلزم ضبط المقياس/المحور بعد الاستيراد بحسب إصدار مستورد glTF.

## Audio source files

ملفات WAV في `Audio/` من الأرشيف الذي رفعه المستخدم. مسارات الفئات الأصلية محفوظة: `brakes` و`collisions` و`engine` و`music` و`tires` و`turbo` و`ui`. لم يتضمن الأرشيف بيان ترخيص منفصلًا للصوت؛ تحقّق من الحقوق قبل إعادة التوزيع التجاري. هذه ملفات مصدر وليست `.uasset`، ولم يربط نموذج C++ الحالي الصوت بمشغلات فعلية.

## الخط العربي

`Fonts/NotoSansArabic-Regular.ttf` و`Fonts/NotoSansArabic-Bold.ttf` من حزمة Ubuntu `fonts-noto-core`، ونص SIL Open Font License 1.1 الكامل موجود في `Fonts/OFL.txt`. هذه ملفات مصدر؛ استوردها إلى `/Game/Fonts/` إذا لم يعرض الخط الافتراضي في UE العربية بصورة مناسبة.
