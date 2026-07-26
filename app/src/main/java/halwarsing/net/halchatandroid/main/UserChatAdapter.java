package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;

import java.util.ArrayList;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCUser;

//Адаптер сообщения в чате
public class UserChatAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private List<HCUser> usersList;
    private UsersChatActivity activity;
    private HalChat hc;

    private static final int TYPE_COMMON=1;
    private static final int TYPE_ADMIN=2;

    private static final String TAG="UCAdapter";

    public UserChatAdapter(List<HCUser> usersList,UsersChatActivity activity,HalChat hc) {
        this.activity=activity;
        this.usersList =usersList;
        this.hc=hc;
    }

    public void addUser(HCUser user) {
        usersList.add(user);
        notifyItemInserted(usersList.size()-1);
    }

    public void replaceUsers(List<HCUser> users) {
        int oldSize=usersList.size();
        usersList=new ArrayList<>(users);
        if(oldSize>0) {
            notifyItemRangeRemoved(0,oldSize);
        }
        if(!usersList.isEmpty()) {
            notifyItemRangeInserted(0,usersList.size());
        }
    }

    @Override
    public int getItemViewType(int position) {
        return usersList.get(position).permissions==0?TYPE_COMMON:TYPE_ADMIN;
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        if(viewType==TYPE_COMMON) {
            View view=LayoutInflater.from(parent.getContext()).inflate(R.layout.chat_user_item_common,parent,false);
            return new CommonUserViewHolder(view);
        }
        View view=LayoutInflater.from(parent.getContext()).inflate(R.layout.chat_user_item_admin,parent,false);
        return new AdminUserViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        HCUser user= usersList.get(position);
        if(holder instanceof CommonUserViewHolder) {
            CommonUserViewHolder userv=(CommonUserViewHolder)holder;
            bindUser(holder,user,userv.icon,userv.nickname,userv.panel);
        } else if (holder instanceof AdminUserViewHolder) {
            AdminUserViewHolder userv=(AdminUserViewHolder) holder;
            bindUser(holder,user,userv.icon,userv.nickname,userv.panel);
        }
    }

    private void bindUser(
            RecyclerView.ViewHolder holder,
            HCUser user,
            ImageView icon,
            TextView nickname,
            LinearLayout panel
    ) {
        icon.setImageResource(R.drawable.testicon);
        nickname.setText(user.user==null?"":user.user.nickname);
        panel.setOnClickListener((View v)->{
            Intent intent=new Intent(activity,ChatUserActivity.class);
            intent.putExtra("uid",user.toId);
            intent.putExtra("chatId",user.chatId);
            activity.startActivity(intent);
            activity.overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
        });

        if(user.user==null || user.user.icon==null || user.user.icon.isEmpty()) {
            return;
        }

        hc.hd.getFileById(user.user.icon).thenAccept(file->
                activity.runOnUiThread(()->{
                    int adapterPosition=holder.getAdapterPosition();
                    if(adapterPosition==RecyclerView.NO_POSITION
                            || adapterPosition>=usersList.size()
                            || usersList.get(adapterPosition).toId!=user.toId) {
                        return;
                    }
                    Glide.with(activity)
                            .load(file)
                            .placeholder(R.drawable.testicon)
                            .into(icon);
                })
        );
    }

    @Override
    public int getItemCount() {
        return usersList.size();
    }

    static class AdminUserViewHolder extends RecyclerView.ViewHolder {
        TextView nickname;
        ImageView icon;
        LinearLayout panel;

        public AdminUserViewHolder(@NonNull View itemView) {
            super(itemView);
            nickname = itemView.findViewById(R.id.chat_user_nickname);
            icon = itemView.findViewById(R.id.chat_user_icon);
            panel=itemView.findViewById(R.id.chat_user_panel);
        }
    }

    static class CommonUserViewHolder extends RecyclerView.ViewHolder {
        TextView nickname;
        ImageView icon;
        LinearLayout panel;

        public CommonUserViewHolder(@NonNull View itemView) {
            super(itemView);
            nickname = itemView.findViewById(R.id.chat_user_nickname);
            icon = itemView.findViewById(R.id.chat_user_icon);
            panel=itemView.findViewById(R.id.chat_user_panel);
        }
    }
}
