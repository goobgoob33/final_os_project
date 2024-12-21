## **روش پراسس (process_version.c)**

### مرحله 1: ایجاد ماتریس

تو کد، یه ماتریس ۱۰۰۰ در ۱۰۰۰ داریم که عناصرش با اعداد تصادفی (۰ یا ۱) مقداردهی می‌شن. از تابع `rand()` استفاده شده و با `srand(time(NULL))` مقدار اولیه تصادفی‌ش تنظیم شده که هر بار خروجی جدید بده.

```c
for (int i = 0; i < 1000; i++) {
    for (int j = 0; j < 1000; j++) {
        matrix[i][j] = rand() % 2;
    }
}
```

### مرحله 2: ساختن FIFO یا پایپ (لوله ارتباطی)

یه فایل پایپ تو مسیر `/tmp/my_pipe` ساخته شده تا داده‌ها بین پراسس رد و بدل بشه. اینطوری هر فرآیند تعداد ۱های خودش رو می‌فرسته.

```c
mkfifo(PIPE_NAME, 0666);
```

### مرحله 3: ایجاد پراسس با `fork()`

حالا ۱۰۰۰ فرآیند تولید می‌شه؛ هرکدوم مسئول شمارش تعداد ۱های **یک ستون خاص** از ماتریسه. اینجا `fork()` استفاده شده که هر بار یه بچه‌فرآیند از والد تولید کنه.

```c
pid_t pid = fork();
if (pid == 0) {

}
```

### مرحله 4: کار بچه‌پراسس

هر بچه تعداد ۱های ستون خودش رو حساب می‌کنه و نتیجه رو داخل اون پایپ می‌فرسته. بعدش با `exit(0)` کارش تموم می‌شه.

```c
int my_count = 0;
for (int j = 0; j < 1000; j++) {
    if (matrix[j][i] == 1) {
        my_count++;
    }
}
int pipe_fd = open(PIPE_NAME, O_WRONLY);
write(pipe_fd, &my_count, sizeof(int));
close(pipe_fd);
exit(0);
```

### مرحله 5: جمع‌آوری نتایج

فرآیند والد منتظر می‌مونه همه بچه‌ها تموم بشن. بعد نتایج رو از پایپ می‌خونه و جمع کل ۱ها رو حساب می‌کنه.

```c
int pipe_fd = open(PIPE_NAME, O_RDONLY);
for (int i = 0; i < 1000; i++) {
    read(pipe_fd, &temp, sizeof(int));
    total += temp;
}
```

### مرحله 6: تمیزکاری

در آخر پایپ بسته و حذف می‌شه، و تعداد کل ۱ها همراه با زمان اجرا چاپ می‌شه.

---

## **روش تردها (thread_version.c)**

### مرحله 1: ساخت ماتریس

دقیقاً مثل روش قبلی یه ماتریس ۱۰۰۰ در ۱۰۰۰ با مقادیر ۰ و ۱ ساخته می‌شه. کدش هم همونه.

### مرحله 2: تعریف متغیر اشتراکی

یه متغیر به اسم `shared` داریم که تعداد کل ۱ها رو نگه می‌داره. برای جلوگیری از مشکل **دسترسی همزمان**، از یه سمافور به اسم `my_semaphore` استفاده شده.

```c
long long shared = 0;
sem_t my_semaphore;
sem_init(&my_semaphore, 0, 1);
```

### مرحله 3: ساخت تردها

۱۰۰۰ ترد ساخته می‌شه که هرکدوم تعداد ۱های **یک ردیف خاص** رو حساب می‌کنه. به هر ترد یه ID داده می‌شه که شماره ردیفش رو مشخص می‌کنه.

```c
for (int i = 0; i < 1000; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, count_ones, &thread_ids[i]);
}
```

### مرحله 4: کار تردها

هر ترد تعداد ۱های ردیف خودش رو می‌شمره و با گرفتن سمافور، مقدارش رو به `shared` اضافه می‌کنه.

```c
sem_wait(&my_semaphore);
shared += local_count;
sem_post(&my_semaphore);
```

### مرحله 5: منتظر موندن برای تردها

برنامه اصلی منتظر می‌مونه همه تردها کارشون رو تموم کنن و بعد نتیجه رو چاپ می‌کنه.

```c
for (int i = 0; i < 1000; i++) {
    pthread_join(threads[i], NULL);
}
```

### **جمع بندی**

- تردها سبک‌تر و سریع‌ترن، پراسس سنگین‌تر ولی ایزوله‌ترن.
- روش تردها معمولاً بهتره مگه اینکه واقعاً نیاز به ایزوله کردن یا توزیع پردازش بین چند سیستم باشه.
- اگه ماتریس بزرگ بشه و ترد/پراسس خیلی زیاد بشن، ممکنه سیستم کرش کنه
