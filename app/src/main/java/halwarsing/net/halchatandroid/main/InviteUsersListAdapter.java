package halwarsing.net.halchatandroid.main;

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

import java.util.ArrayList;
import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.SearchUser;

public class InviteUsersListAdapter extends RecyclerView.Adapter<InviteUsersListAdapter.InviteUserListViewHolder> {
    private List<SearchUser> usersList;
    private InviteChatActivity invChatActivity;
    private HalChat hc;
    private static final String TAG="IULA";
    public InviteUsersListAdapter(InviteChatActivity invChatActivity,HalChat hc) {
        this.invChatActivity=invChatActivity;
        this.hc=hc;
        usersList=new ArrayList<>();
    }

    @NonNull
    @Override
    public InviteUserListViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view= LayoutInflater.from(parent.getContext()).inflate(R.layout.search_user_item,parent,false);
        return new InviteUserListViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull InviteUserListViewHolder holder, int position) {
        SearchUser user=usersList.get(position);
        holder.username.setText(user.user.nickname);

        hc.hd.getFileById(user.user.icon).thenAccept(fileIcon->{
            new Handler(Looper.getMainLooper()).post(() -> {
                Glide.with(invChatActivity)
                        .load(fileIcon)
                        .override(150, 150)
                        .placeholder(R.drawable.ic_add_people)
                        .into(holder.icon);
            });
        });

        holder.itemView.setOnClickListener(v->{
            invChatActivity.inviteUser(user.user);
        });
    }

    @Override
    public int getItemCount() {
        return usersList.size();
    }

    public void removeAll() {
        usersList.clear();
        notifyDataSetChanged();
    }

    public void updateUsers(List<SearchUser> users) {
        usersList=users;
        notifyDataSetChanged();
    }

    public static class InviteUserListViewHolder extends RecyclerView.ViewHolder {
        TextView username;
        ImageView icon;

        public InviteUserListViewHolder(@NonNull View itemView) {
            super(itemView);
            username=itemView.findViewById(R.id.username);
            icon=itemView.findViewById(R.id.userIcon);
        }
    }
}
