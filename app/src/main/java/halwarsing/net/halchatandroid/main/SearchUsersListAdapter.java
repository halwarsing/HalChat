package halwarsing.net.halchatandroid.main;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.SearchUser;

public class SearchUsersListAdapter extends RecyclerView.Adapter<SearchUsersListAdapter.SearchUserListViewHolder> {
    private List<SearchUser> usersList;
    private SearchPeopleActivity peopleActivity;
    private static final String TAG="SULA";

    public SearchUsersListAdapter(List<SearchUser> usersList,SearchPeopleActivity peopleActivity) {
        this.usersList=usersList;
        this.peopleActivity=peopleActivity;
    }

    @NonNull
    @Override
    public SearchUserListViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view= LayoutInflater.from(parent.getContext()).inflate(R.layout.search_user_item,parent,false);
        return new SearchUserListViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull SearchUserListViewHolder holder, int position) {
        SearchUser user=usersList.get(position);
        holder.username.setText(user.user.nickname);
        holder.icon.setImageBitmap(user.icon);
        holder.itemView.setOnClickListener(v->{
            peopleActivity.openUser(user.user);
        });
    }

    @Override
    public int getItemCount() {
        return usersList.size();
    }

    public void removeAll() {
        usersList.clear();
        Log.e(TAG,"REMOVE ALL");
        notifyDataSetChanged();
    }

    public static class SearchUserListViewHolder extends RecyclerView.ViewHolder {
        TextView username;
        ImageView icon;

        public SearchUserListViewHolder(@NonNull View itemView) {
            super(itemView);
            username=itemView.findViewById(R.id.username);
            icon=itemView.findViewById(R.id.userIcon);
        }

    }

    protected void addNewUser(SearchUser user) {
        usersList.add(user);
        Log.e(TAG,"USER: "+user.user.nickname+":"+(usersList.size()-1));
        notifyItemInserted(usersList.size()-1);
    }
}
