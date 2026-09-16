#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/sched/cputime.h>
#include <linux/timekeeping.h>

#define PROC_NAME "continfo_pr2_so1_202503517"
#define CMDLINE_SIZE 256

static struct proc_dir_entry *proc_entry;

static void get_task_cmdline(struct task_struct *task, char *buffer)
{
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long length;
    unsigned long i;
    int copied;

    buffer[0] = '\0';

    if (task->mm == NULL)
        return;

    arg_start = task->mm->arg_start;
    arg_end = task->mm->arg_end;

    if (arg_end <= arg_start)
        return;

    length = arg_end - arg_start;

    if (length >= CMDLINE_SIZE)
        length = CMDLINE_SIZE - 1;

    copied = access_process_vm(
        task,
        arg_start,
        buffer,
        length,
        0
    );

    if (copied <= 0) {
        buffer[0] = '\0';
        return;
    }

    buffer[copied] = '\0';

    /*
     * La línea de comandos de Linux utiliza '\0'
     * entre sus argumentos. Los convertimos a espacios.
     */
    for (i = 0; i < copied; i++) {
        if (buffer[i] == '\0')
            buffer[i] = ' ';
    }

    /*
     * Eliminamos espacios finales.
     */
    while (copied > 0 && buffer[copied - 1] == ' ') {
        buffer[copied - 1] = '\0';
        copied--;
    }
}

static int continfo_show(struct seq_file *m, void *v)
{
    struct sysinfo info;
    struct task_struct *task;

    unsigned long total_ram;
    unsigned long free_ram;
    unsigned long used_ram;
    unsigned long rss_kb;
    unsigned long vsz_kb;

    unsigned long mem_percent_int;
    unsigned long mem_percent_dec;

    unsigned long cpu_percent_int;
    unsigned long cpu_percent_dec;

    u64 current_time_ns;
    u64 elapsed_process_time_ns;

    u64 utime_ns;
    u64 stime_ns;
    u64 cpu_time_ns;

    char cmdline[CMDLINE_SIZE];

    si_meminfo(&info);

    current_time_ns = ktime_get_ns();

    total_ram = info.totalram * info.mem_unit;
    free_ram = info.freeram * info.mem_unit;
    used_ram = total_ram - free_ram;

    seq_printf(m, "Sonda de Kernel - Proyecto 2 SO1\n");
    seq_printf(m, "RAM Total: %lu KB\n", total_ram / 1024);
    seq_printf(m, "RAM Libre: %lu KB\n", free_ram / 1024);
    seq_printf(m, "RAM Usada: %lu KB\n", used_ram / 1024);
    seq_printf(m, "\n");

    seq_printf(
        m,
        "PID\tNOMBRE\tCMDLINE\tVSZ(KB)\tRSS(KB)\tMEM(%%)\tCPU(%%)\n"
    );

    for_each_process(task) {

        if (task->mm == NULL)
            continue;

        vsz_kb = (task->mm->total_vm * PAGE_SIZE) / 1024;
        rss_kb = (get_mm_rss(task->mm) * PAGE_SIZE) / 1024;

        /*
         * Obtener línea de comandos.
         */
        get_task_cmdline(task, cmdline);

        /*
         * CPU acumulado del proceso desde su inicio.
         */
        task_cputime_adjusted(task, &utime_ns, &stime_ns);
        cpu_time_ns = utime_ns + stime_ns;

        /*
         * Tiempo que lleva vivo el proceso.
         */
        if (current_time_ns > task->start_time)
            elapsed_process_time_ns =
                current_time_ns - task->start_time;
        else
            elapsed_process_time_ns = 0;

        /*
         * CPU% = tiempo de CPU utilizado /
         *        tiempo de vida del proceso * 100
         */
        if (elapsed_process_time_ns > 0) {

            cpu_percent_int =
                (cpu_time_ns * 100) /
                elapsed_process_time_ns;

            cpu_percent_dec =
                ((cpu_time_ns * 10000) /
                 elapsed_process_time_ns) % 100;

        } else {

            cpu_percent_int = 0;
            cpu_percent_dec = 0;
        }

        /*
         * Porcentaje de RAM utilizado por el proceso.
         */
        if (total_ram > 0) {

            mem_percent_int =
                (rss_kb * 10000) /
                (total_ram / 1024);

            mem_percent_dec =
                mem_percent_int % 100;

            mem_percent_int /= 100;

        } else {

            mem_percent_int = 0;
            mem_percent_dec = 0;
        }

        seq_printf(
            m,
            "%d\t%s\t%s\t%lu\t%lu\t%lu.%02lu%%\t%lu.%02lu%%\n",
            task->pid,
            task->comm,
            cmdline,
            vsz_kb,
            rss_kb,
            mem_percent_int,
            mem_percent_dec,
            cpu_percent_int,
            cpu_percent_dec
        );
    }

    return 0;
}

static int continfo_open(struct inode *inode, struct file *file)
{
    return single_open(file, continfo_show, NULL);
}

static const struct proc_ops continfo_ops = {
    .proc_open = continfo_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init continfo_init(void)
{
    proc_entry = proc_create(
        PROC_NAME,
        0444,
        NULL,
        &continfo_ops
    );

    if (!proc_entry) {
        printk(KERN_ERR
               "continfo_pr2_so1: error al crear /proc\n");
        return -ENOMEM;
    }

    printk(KERN_INFO
           "continfo_pr2_so1: modulo cargado\n");

    return 0;
}

static void __exit continfo_exit(void)
{
    proc_remove(proc_entry);

    printk(KERN_INFO
           "continfo_pr2_so1: modulo descargado\n");
}

module_init(continfo_init);
module_exit(continfo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Saloj");
MODULE_DESCRIPTION(
    "Sonda de Kernel para telemetria de contenedores - Proyecto 2 SO1"
);
