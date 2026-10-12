package halwarsing.net.halchatandroid.views;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

import org.json.JSONObject;

import java.util.Locale;

import halwarsing.net.halchatandroid.R;

//Port of LoadingExperience.js (window.HalChatAILoader)
//create/update/stop can be called from any thread, all work goes to UI thread
public class AILoading {
    private static final String[] FACTS=new String[]{
        "Модель запускается прямо в браузере — сообщения не нужно отправлять во внешний ИИ-сервис.",
        "Первый запуск обычно самый долгий: загруженные файлы сохраняются в браузере для следующих входов.",
        "После скачивания браузеру нужно ещё немного времени, чтобы проверить файлы и запустить модель.",
        "Скорость зависит не только от интернета, но и от мощности устройства и свободной памяти.",
        "Оставьте страницу открытой до завершения: закрытие вкладки прервёт текущий запуск."
    };
    private static final String[] STEPS=new String[]{"Подготовка","Файлы модели","Запуск ИИ"};

    /*state*/
    private boolean active=false;
    private long startedAt=0;
    private long lastProgressAt=0;
    private int factIndex=0;
    private int fileCount=0;
    private String historyKey="";
    private long previousDuration=0;
    private double smoothedEta=-1;
    private boolean sawProgress=false;
    private String phase="preparing";

    private final View root;
    private final Handler handler=new Handler(Looper.getMainLooper());
    private final SharedPreferences prefs;
    private final TextView stage,status,progressText,fileCounter,loadFileName,elapsedText,eta,factText;
    private final TextView[] steps;
    private final ProgressBar progressBar;

    private final Runnable elapsedTimer=new Runnable() {
        @Override
        public void run() {
            updateElapsed();
            handler.postDelayed(this,1000);
        }
    };

    private final Runnable factTimer=new Runnable() {
        @Override
        public void run() {
            rotateFact();
            handler.postDelayed(this,8000);
        }
    };

    //root - inflated R.layout.ai_loading_view
    public AILoading(View root) {
        this.root=root;
        prefs=root.getContext().getSharedPreferences("halchat-ai-load-duration",Context.MODE_PRIVATE);

        stage=root.findViewById(R.id.LLLMHSMStage);
        status=root.findViewById(R.id.LLLMHSMStatus);
        progressBar=root.findViewById(R.id.LLLMHSMProgressDiv);
        progressText=root.findViewById(R.id.LLLMHSMProgressText);
        fileCounter=root.findViewById(R.id.LLLMHSMFileCounter);
        loadFileName=root.findViewById(R.id.LLLMHSMLoadFileName);
        elapsedText=root.findViewById(R.id.LLLMHSMElapsed);
        eta=root.findViewById(R.id.LLLMHSMEta);
        factText=root.findViewById(R.id.LLLMHSMFactText);
        steps=new TextView[]{
            root.findViewById(R.id.LLLMHSMStep1),
            root.findViewById(R.id.LLLMHSMStep2),
            root.findViewById(R.id.LLLMHSMStep3)
        };
    }

    private static double clamp(double value, double min, double max) {
        return Math.min(max,Math.max(min,value));
    }

    private static String formatDuration(double totalSeconds) {
        long seconds=Math.max(0,Math.round(totalSeconds));
        if(seconds<60)return seconds+" сек";

        long minutes=seconds/60;
        long rest=seconds%60;
        if(minutes<60)return rest>0?minutes+" мин "+rest+" сек":minutes+" мин";

        long hours=minutes/60;
        long restMinutes=minutes%60;
        return restMinutes>0?hours+" ч "+restMinutes+" мин":hours+" ч";
    }

    private static String formatClock(double totalSeconds) {
        long seconds=Math.max(0,(long)Math.floor(totalSeconds));
        return String.format(Locale.US,"%d:%02d",seconds/60,seconds%60);
    }

    private static String formatBytes(long value) {
        if(value<=0)return "";

        String[] units=new String[]{"Б","КБ","МБ","ГБ"};
        int unitIndex=Math.min((int)Math.floor(Math.log(value)/Math.log(1024)),units.length-1);
        double normalized=value/Math.pow(1024,unitIndex);
        int digits=normalized>=100||unitIndex==0?0:1;
        return String.format(Locale.US,"%."+digits+"f",normalized)+" "+units[unitIndex];
    }

    private static String formatFilename(String filename) {
        if(filename==null||filename.isEmpty())return "";
        String[] f=filename.split("[\\\\/]");
        String clean=f.length==0?"":f[f.length-1].replaceAll("[?#].*$","");
        if(clean.length()<=42)return clean;
        return clean.substring(0,21)+"…"+clean.substring(clean.length()-16);
    }

    //localStorage -> SharedPreferences, value in seconds
    private long readPreviousDuration(String key) {
        if(key.isEmpty())return 0;
        long duration=prefs.getLong(key,0);
        return duration>0&&duration<3600?duration:0;
    }

    private void saveDuration() {
        if(historyKey.isEmpty()||!sawProgress)return;
        double duration=(SystemClock.elapsedRealtime()-startedAt)/1000.0;
        if(duration<1||duration>=3600)return;

        long previous=readPreviousDuration(historyKey);
        long smoothed=previous>0?Math.round(previous*0.65+duration*0.35):Math.round(duration);
        prefs.edit().putLong(historyKey,smoothed).apply();
    }

    private boolean isOnline() {
        ConnectivityManager cm=(ConnectivityManager)root.getContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        return cm==null||cm.getActiveNetwork()!=null;
    }

    private void setStep(int activeIndex) {
        for(int i=0;i<steps.length;i++) {
            steps[i].setText((i<activeIndex?"✓":String.valueOf(i+1))+" "+STEPS[i]);
            steps[i].setAlpha(i<=activeIndex?1.0f:0.4f);
            steps[i].setTextColor(root.getContext().getColor(i==activeIndex?R.color.accent1:R.color.textColor));
        }
    }

    private void setProgress(double overall) {
        progressBar.setIndeterminate(false);
        progressBar.setProgress((int)Math.round(overall*10));
    }

    private void updateEta(double overallProgress, String etaPhase) {
        double elapsed=Math.max(0.1,(SystemClock.elapsedRealtime()-startedAt)/1000.0);

        if(etaPhase.equals("initializing")) {
            if(previousDuration>elapsed+2) {
                eta.setText("Осталось примерно "+formatDuration(previousDuration-elapsed));
            } else {
                eta.setText("Файлы готовы — запускаем модель");
            }
            return;
        }

        double estimate=-1;
        if(overallProgress>=3&&overallProgress<100&&elapsed>=2) {
            estimate=elapsed*(100-overallProgress)/overallProgress;
        }

        if(previousDuration>elapsed) {
            double historicalEstimate=previousDuration-elapsed;
            estimate=estimate<0?historicalEstimate:estimate*0.45+historicalEstimate*0.55;
        }

        if(estimate<0||Double.isNaN(estimate)||Double.isInfinite(estimate)) {
            eta.setText("Оцениваем оставшееся время…");
            return;
        }

        estimate=clamp(estimate,1,3599);
        smoothedEta=smoothedEta<0?estimate:smoothedEta*0.72+estimate*0.28;
        eta.setText("Осталось примерно "+formatDuration(smoothedEta));
    }

    private void updateElapsed() {
        if(!active)return;

        long now=SystemClock.elapsedRealtime();
        double elapsed=(now-startedAt)/1000.0;
        elapsedText.setText(formatClock(elapsed));

        if(!sawProgress) {
            if(elapsed>=20) {
                status.setText(isOnline()
                        ?"Подготовка занимает дольше обычного, но процесс продолжается."
                        :"Нет соединения с сетью. Вернитесь в онлайн — загрузка продолжится.");
            } else if(elapsed>=7) {
                status.setText("Проверяем сохранённые файлы и готовим загрузку…");
            }
        } else if(now-lastProgressAt>15000&&phase.equals("downloading")) {
            status.setText("Получаем следующий фрагмент. На медленной сети паузы между обновлениями нормальны.");
        }
    }

    private void rotateFact() {
        factIndex=(factIndex+1)%FACTS.length;
        factText.setText(FACTS[factIndex]);
    }

    //options: {fileCount, historyKey}
    private void configure(JSONObject options) {
        if(options==null)return;

        int count=options.optInt("fileCount",0);
        if(count>0) {
            fileCount=count;
            fileCounter.setText(fileCount==1?"1 файл":fileCount+" файлов");
        }

        String key=options.optString("historyKey","");
        if(!key.isEmpty()) {
            key=key.replaceAll("[^a-zA-Z0-9_.:-]","_");
            historyKey=key.length()>120?key.substring(0,120):key;
            previousDuration=readPreviousDuration(historyKey);
            if(previousDuration>0) {
                eta.setText("В прошлый раз: около "+formatDuration(previousDuration));
            }
        }
    }

    private void createUI(JSONObject options) {
        if(active) {
            configure(options);
            return;
        }

        active=true;
        startedAt=SystemClock.elapsedRealtime();
        lastProgressAt=startedAt;
        factIndex=0;
        fileCount=0;
        historyKey="";
        previousDuration=0;
        smoothedEta=-1;
        sawProgress=false;
        phase="preparing";

        //Reset view (it can be reused after stop)
        stage.setText("Подготавливаем ИИ");
        status.setText("Проверяем браузер и сохранённые файлы…");
        progressBar.setIndeterminate(true);
        progressBar.setProgress(0);
        progressText.setText("Подготовка…");
        fileCounter.setText("Считаем файлы…");
        loadFileName.setText("");
        setStep(0);
        elapsedText.setText("0:00");
        eta.setText("Оцениваем оставшееся время…");
        factText.setText(FACTS[0]);
        root.setVisibility(View.VISIBLE);

        configure(options);

        handler.postDelayed(elapsedTimer,1000);
        handler.postDelayed(factTimer,8000);
    }

    //progress: {phase, pct, loaded, total, fromCache, filename, fileIndex, fileCount, scope}
    private void updateUI(JSONObject data) {
        if(!active)createUI(null);

        String curPhase=data.optString("phase","downloading");
        String scope=data.optString("scope","");
        boolean isRag=scope.equals("rag");
        boolean fromCache=data.optBoolean("fromCache",false);
        double pct=data.optDouble("pct",0);
        pct=Double.isNaN(pct)?0:clamp(pct,0,100);
        int incomingCount=data.optInt("fileCount",0);
        int count=incomingCount>0?incomingCount:Math.max(1,fileCount);
        int fileIndex=(int)clamp(data.optInt("fileIndex",0),0,count-1);
        boolean isFinalInitialization=curPhase.equals("initializing")&&!isRag;
        double overall=isFinalInitialization?100:clamp((fileIndex+pct/100)/count*100,0,100);

        fileCount=count;
        lastProgressAt=SystemClock.elapsedRealtime();
        sawProgress=true;
        phase=curPhase;

        if(curPhase.equals("initializing")) {
            setProgress(overall);
            stage.setText(isRag?"Подготавливаем память ИИ":"Запускаем ИИ");
            status.setText(isRag
                    ?"Файлы дополнительной памяти готовы, настраиваем поиск…"
                    :"Файлы готовы. Проверяем модель и выделяем память…");
            progressText.setText(isRag?"Общий прогресс "+Math.round(overall)+"%":"Файлы готовы");
            fileCounter.setText(isRag?(fileIndex+1)+" из "+count:count+" из "+count);
            loadFileName.setText("");
            setStep(isRag?1:2);
            updateEta(overall,isRag?"downloading":"initializing");
            return;
        }

        boolean hasExactProgress=pct>0||fileIndex>0||fromCache;
        if(hasExactProgress)setProgress(overall);
        else progressBar.setIndeterminate(true);

        stage.setText(isRag?"Загружаем память ИИ":"Загружаем файлы модели");
        status.setText(fromCache
                ?"Файл уже сохранён в браузере — используем локальную копию."
                :pct>0
                    ?"Загрузка идёт. Можно дождаться здесь."
                    :"Подключаемся к хранилищу и запрашиваем файл…");

        progressText.setText(hasExactProgress?"Общий прогресс "+Math.round(overall)+"%":"Получаем данные…");
        fileCounter.setText(pct>0||fromCache
                ?"Файл "+(fileIndex+1)+" из "+count+" · "+Math.round(pct)+"%"
                :"Файл "+(fileIndex+1)+" из "+count+" · подключаемся");

        String loadedText=formatBytes(data.optLong("loaded",0));
        String totalText=formatBytes(data.optLong("total",0));
        String sizeText=!loadedText.isEmpty()&&!totalText.isEmpty()
                ?" · "+loadedText+" из "+totalText
                :!loadedText.isEmpty()?" · получено "+loadedText:"";
        String filename=formatFilename(data.optString("filename",""));
        loadFileName.setText((filename.isEmpty()?"Файл модели":filename)+sizeText);
        setStep(1);
        updateEta(overall,curPhase);
    }

    private void cleanup(boolean completed) {
        if(!active)return;
        if(completed)saveDuration();

        handler.removeCallbacks(elapsedTimer);
        handler.removeCallbacks(factTimer);
        active=false;
    }

    private void stopUI(boolean completed) {
        if(!active)return;
        cleanup(completed);
        root.setVisibility(View.GONE);
    }

    public void create(JSONObject options) {
        handler.post(()->createUI(options));
    }

    public void update(JSONObject progress) {
        if(progress==null)return;
        handler.post(()->updateUI(progress));
    }

    //completed=true saves load duration for next ETA (false - load was interrupted)
    public void stop(boolean completed) {
        handler.post(()->stopUI(completed));
    }

    public void stop() {
        stop(true);
    }
}
