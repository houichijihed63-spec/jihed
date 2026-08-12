# Invoice Extractor (Qt / C++)

تطبيق سطح مكتب بـ Qt Widgets يعيد نفس النظام الموصوف: تحميل صورة فاتورة، قراءتها عبر
**OCR.space API**، استخراج الحقول والجدول، حساب المجاميع، ثم حفظها في قاعدة بيانات
SQLite محلية وعرضها في لوحة تحكم مع بحث وفلاتر — يطابق الشاشتين المرفقتين
(`Tableau de bord` و `Extraction intelligente de factures`).

## البنية

```
src/
  core/
    InvoiceModel.h        // بنية بيانات الفاتورة والبنود
    TextNormalizer.*       // تنظيف نص OCR (إزالة النقاط، تصحيح أخطاء شائعة)
    FieldExtractor.*        // استخراج رقم الفاتورة، التاريخ، المورد، العميل...
    TableExtractor.*        // إعادة بناء جدول البنود من إحداثيات الكلمات
    InvoiceCalculator.*      // cleanNumber() + حساب subtotal/tax/total
    OcrClient.*               // استدعاء OCR.space عبر QNetworkAccessManager
    Database.*                 // تخزين SQLite (QtSql)
  ui/
    MainWindow.*        // الشريط الجانبي + التنقل بين الصفحتين
    Sidebar.*            // شعار الشركة + أزرار Tableau de bord / Nouvelle facture
    StatCard.*            // بطاقات الإحصائيات الثلاث
    DashboardPage.*        // لوحة التحكم: بطاقات + فلاتر + جدول الفواتير
    NewInvoicePage.*         // رفع الصورة -> استخراج -> مراجعة -> حفظ
    DropArea.*                 // منطقة السحب والإفلات لاختيار الصورة
resources/style.qss     // تنسيق بصري (ألوان، بطاقات مدورة، إلخ)
```

## المتطلبات

- Qt 6 (أو Qt 5.15+) بمكوّنات: `Widgets`, `Network`, `Sql`
- مترجم C++17 (GCC/Clang/MSVC)
- CMake ≥ 3.16
- سائق SQLite لِـ Qt (`QSQLITE` — مضمّن افتراضيًا مع معظم توزيعات Qt)
- مفتاح **مجاني** من [ocr.space](https://ocr.space/ocrapi) — سيُطلب منك إدخاله عند أول
  عملية استخراج، ويُحفظ محليًا عبر `QSettings` (لا حاجة لتعديل الكود).

## البناء والتشغيل

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
./InvoiceExtractor        # أو InvoiceExtractor.exe على ويندوز
```

## ملاحظات على منطق الاستخراج

- **الجدول**: `TableExtractor` يبحث أولًا عن كلمات ترويسة الأعمدة
  (Désignation/Qté/Unité/P.U/Total) لتحديد مواقعها الأفقية (X)، ثم يجمّع بقية
  الكلمات في صفوف حسب التقارب العمودي (Y)، ويُسند كل كلمة لأقرب عمود.
- **المجاميع**: لا يُعتمد على رقم "Total" المقروء من الصورة مباشرة؛ بل يُحسب
  `subtotal` من مجموع بنود الجدول، ثم `tax_amount = subtotal × taxRate/100`
  (بنسبة TVA المستخرجة من النص، أو 19% افتراضيًا)، ثم `total = subtotal + tax`.
- **الأرقام الفرنسية/التونسية**: `InvoiceCalculator::cleanNumber()` يتعامل مع
  الفاصلة كفاصل عشري، والمسافة أو النقطة كفاصل آلاف، حسب النمط المكتشف.
- **المراجعة قبل الحفظ**: بعد الاستخراج، تُعرض شاشة مراجعة قابلة للتعديل (حقول +
  جدول بنود) قبل الحفظ النهائي في SQLite — لتفادي الاعتماد الكامل على دقة OCR.

## التخصيص السريع

- **تغيير اسم/شعار الشركة**: `MainWindow::MainWindow()` →
  `m_sidebar->setCompanyInfo("ELFOULADH", "Menzel Bourguiba")`.
- **فاتورة نموذجية**: زر "Utiliser une facture exemple" جاهز في الواجهة؛ لتفعيله
  ضع صورة نموذجية في `resources/` وحمّلها في `NewInvoicePage::buildUi()` بدل
  رسالة "لم يتم إرفاق فاتورة نموذجية بعد".
- **عملة أخرى غير TND**: عدّل `Invoice::currency` الافتراضية في `InvoiceModel.h`.
