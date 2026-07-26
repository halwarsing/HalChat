package halwarsing.net.halchatandroid.main;

import static halwarsing.net.halchatandroid.main.HalChatFunctionsLib.getTimeFromSeconds;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCChat;
import halwarsing.net.halchatandroid.type.HCMessage;

//Список чатов в интерфейсе
public class ChatListAdapter extends RecyclerView.Adapter<ChatListAdapter.ChatListViewHolder> {
    private List<ChatInfoList> chatList;
    private MainActivity mainActivity;
    private HalChat hc;
    private static final String TAG = "CLA";

    public ChatListAdapter(List<ChatInfoList> chatList,MainActivity mainActivity, HalChat hc) {
        this.chatList=chatList;
        this.mainActivity=mainActivity;
        this.hc=hc;
        Collections.sort(chatList, (c1, c2) -> -Long.compare(c2.getSecTime(), c1.getSecTime()));
    }

    @NonNull
    @Override
    public ChatListViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view= LayoutInflater.from(parent.getContext()).inflate(R.layout.item_chat,parent,false);
        return new ChatListViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ChatListViewHolder holder, int position) {

        ChatInfoList chat=chatList.get(position);
        holder.chatName.setText(chat.getName());
        holder.iconChat.setImageResource(R.drawable.ic_add_people);
        HalChatGroupChats.HCEncryptedMessage lmsg=chat.getLastMessage();


        if(lmsg.msg!=null&&lmsg.msg.type==2) {
            hc.hnUsers.getUserByUserId(lmsg.msg.fromId).thenAccept(fromUser-> {
                mainActivity.runOnUiThread(() -> {
                    if(fromUser!=null && isHolderBoundTo(holder,chat.getUid())) {
                        holder.lastMessage.setText("Присоединился ~"+fromUser.nickname);
                    }
                });
            });

        } else if(lmsg.msg!=null&&lmsg.msg.type==3) {
            hc.hnUsers.getUserByUserId(lmsg.msg.fromId).thenAccept(fromUser-> {
                mainActivity.runOnUiThread(() -> {
                    if(fromUser!=null && isHolderBoundTo(holder,chat.getUid())) {
                        holder.lastMessage.setText("Вышел ~"+fromUser.nickname);
                    }
                });
            });
        } else {
            holder.lastMessage.setText(HalChatFunctionsLib.replaceEmojis(mainActivity.getApplicationContext(),holder.lastMessage,hc, lmsg.getMessage(hc)),TextView.BufferType.SPANNABLE);
        }

        holder.timeChat.setText(chat.getTime());

        hc.hd.getFileById(chat.getIcon()).thenAccept(fileIcon->{
            new Handler(Looper.getMainLooper()).post(() -> {
                if(isHolderBoundTo(holder,chat.getUid())) {
                    Glide.with(mainActivity)
                            .load(fileIcon)
                            .override(150, 150)
                            .placeholder(R.drawable.ic_add_people)
                            .into(holder.iconChat);
                }
            });
        }).exceptionally(error -> {
            Log.e(TAG,"Unable to load chat icon "+chat.getIcon(),error);
            return null;
        });

        holder.itemView.setOnClickListener(v->{
            mainActivity.openChat(chat.getUid(),chat.getName());
        });
    }

    @Override
    public int getItemCount() {
        return chatList.size();
    }

    private boolean isHolderBoundTo(ChatListViewHolder holder,long chatId) {
        int position=holder.getAdapterPosition();
        return position!=RecyclerView.NO_POSITION
                && position<chatList.size()
                && chatList.get(position).getUid()==chatId;
    }

    public static class ChatListViewHolder extends RecyclerView.ViewHolder {
        TextView chatName;
        TextView lastMessage;
        TextView timeChat;
        ImageView iconChat;

        public ChatListViewHolder(@NonNull View itemView) {
            super(itemView);
            chatName=itemView.findViewById(R.id.chatName);
            lastMessage=itemView.findViewById(R.id.chatLastMsg);
            timeChat=itemView.findViewById(R.id.chatLastMsgTime);
            iconChat=itemView.findViewById(R.id.chatIcon);
        }

    }

    protected void addChat(ChatInfoList chatInfoList) {
        this.chatList.add(chatInfoList);

        notifyItemInserted(this.chatList.size()-1);
    }

    protected void addNewChat(HCChat chat, HCMessage lastMsg) {
        for (int i=0;i<chatList.size();i++) {
            ChatInfoList il=chatList.get(i);
            if(il.getUid()==chat.chatUID) {
                return;
            }
        }

        long time=lastMsg==null?chat.created:lastMsg.time;
        time=System.currentTimeMillis()-time*1000;
        time/=1000;

        String msg="Новый чат";

        if(lastMsg!=null) {
            msg=lastMsg.decryptedMessage;
        } else if(!mainActivity.hc.chatGroupChats.hasPasswordChat(chat.chatUID)) {
            msg="Введите пароль";
        }
        chatList.add(new ChatInfoList(chat.chatUID,chat.name,new HalChatGroupChats.HCEncryptedMessage(msg,false,lastMsg),chat.icon, getTimeFromSeconds((int)time),(int)time));
        notifyItemInserted(chatList.size()-1);
        // Сортировка по времени (сначала самые новые)
        Collections.sort(chatList, new Comparator<ChatInfoList>() {
            @Override
            public int compare(ChatInfoList c1, ChatInfoList c2) {
                return -Long.compare(c2.getSecTime(), c1.getSecTime());
            }
        });

        notifyDataSetChanged(); // Полное обновление списка
    }

    protected void updateChat(HCChat chat,HCMessage lastMsg) {
        int z=-1;

        for (int i=0;i<chatList.size();i++) {
            ChatInfoList il=chatList.get(i);
            if(il.getUid()==chat.chatUID) {
                z=i;
                break;
            }
        }
        if(z==-1)return;
        long time=lastMsg==null?chat.created:lastMsg.time;
        time=System.currentTimeMillis()-time*1000;
        time/=1000;

        String msg="Новый чат";

        if(lastMsg!=null && hc.chatGroupChats.hasPasswordChat(chat.chatUID)) {
            msg=lastMsg.decryptedMessage;
        } else if(!mainActivity.hc.chatGroupChats.hasPasswordChat(chat.chatUID)) {
            msg="Введите пароль";
        }

        chatList.set(z,new ChatInfoList(chat.chatUID,chat.name,new HalChatGroupChats.HCEncryptedMessage(msg,false,lastMsg),chat.icon, getTimeFromSeconds((int)time),(int)time));
        //notifyItemInserted(z);
        // Сортировка по времени (сначала самые новые)
        Collections.sort(chatList, new Comparator<ChatInfoList>() {
            @Override
            public int compare(ChatInfoList c1, ChatInfoList c2) {
                return -Long.compare(c2.getSecTime(), c1.getSecTime());
            }
        });

        mainActivity.runOnUiThread(()->{
            notifyDataSetChanged();
        });
    }

    protected void deleteChat(HCChat chat) {
        for(int i=0;i<chatList.size();i++) {
            ChatInfoList chatInfoList=chatList.get(i);
            if(chatInfoList.getUid()==chat.chatUID) {
                chatList.remove(i);
                notifyItemRemoved(i);
                return;
            }
        }
    }

    protected void onNewMessage(HCChat chat,HCMessage message) {
        for(int i=0;i<chatList.size();i++) {
            ChatInfoList chatInfoList=chatList.get(i);
            if(chatInfoList.getUid()==chat.chatUID) {
                int time=Math.max((int)((System.currentTimeMillis()-message.time*1000)/1000),0);

                String msg=message.decryptedMessage;
                if(!mainActivity.hc.chatGroupChats.hasPasswordChat(chat.chatUID)) {
                    msg="Введите пароль";
                }

                ChatInfoList updatedChat=new ChatInfoList(chat.chatUID,chat.name,new HalChatGroupChats.HCEncryptedMessage(msg,false,message),chat.icon,getTimeFromSeconds(
                        time
                ),time);
                chatList.set(i,updatedChat);

                notifyItemChanged(i);

                Collections.sort(chatList, new Comparator<ChatInfoList>() {
                    @Override
                    public int compare(ChatInfoList c1, ChatInfoList c2) {
                        return -Long.compare(c2.getSecTime(), c1.getSecTime());
                    }
                });

                notifyDataSetChanged(); // Полное обновление списка
                break;
            }
        }
    }
}
