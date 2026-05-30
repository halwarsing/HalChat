package halwarsing.net.halchatandroid.main;

import android.content.Intent;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCUser;

//Адаптер сообщения в чате
public class UserChatAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private List<HCUser> usersList;
    private UsersChatActivity activity;

    private static final int TYPE_COMMON=1;
    private static final int TYPE_ADMIN=2;

    private static final String TAG="UCAdapter";

    public UserChatAdapter(List<HCUser> usersList,UsersChatActivity activity) {
        this.activity=activity;
        this.usersList =usersList;
    }

    public void addUser(HCUser user) {
        usersList.add(user);
        notifyItemInserted(usersList.size()-1);
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
            userv.icon.setImageBitmap(BitmapFactory.decodeFile(user.user.fullPathIcon));
            userv.nickname.setText(user.user.nickname);
            userv.panel.setOnClickListener((View v)->{
                Intent intent=new Intent(activity,ChatUserActivity.class);
                intent.putExtra("uid",user.toId);
                intent.putExtra("chatId",user.chatId);
                activity.startActivity(intent);
                activity.overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
            });
        } else if (holder instanceof AdminUserViewHolder) {
            AdminUserViewHolder userv=(AdminUserViewHolder) holder;
            userv.icon.setImageBitmap(BitmapFactory.decodeFile(user.user.fullPathIcon));
            userv.nickname.setText(user.user.nickname);
            userv.panel.setOnClickListener((View v)->{
                Intent intent=new Intent(activity,ChatUserActivity.class);
                intent.putExtra("uid",user.toId);
                intent.putExtra("chatId",user.chatId);
                activity.startActivity(intent);
                activity.overridePendingTransition(R.anim.slide_in_down,R.anim.stay);
            });
        }
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
