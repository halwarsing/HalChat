package halwarsing.net.halchatandroid.type;

import java.io.File;
import java.util.ArrayList;

public class ShareDataFiles extends ShareData {
    private ArrayList<File> files;

    public ShareDataFiles(ArrayList<File> files) {
        this.files=files;
    }

    public ArrayList<File> getFiles() {
        return files;
    }
}
