package halwarsing.net.halchatandroid.main;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.MimeTypeMap;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.core.content.FileProvider;
import androidx.recyclerview.widget.RecyclerView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;
import com.google.android.flexbox.FlexboxLayout;

import org.json.JSONArray;
import org.json.JSONException;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import halwarsing.net.halchatandroid.R;
import halwarsing.net.halchatandroid.type.HCMessage;
import halwarsing.net.halchatandroid.type.HDFile;
import halwarsing.net.halchatandroid.type.MessageReaction;
import halwarsing.net.halchatandroid.views.RecordedAudioView;
import halwarsing.net.halchatandroid.views.WaveformView;

//Адаптер сообщения в чате
public class MessageAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    protected List<HCMessage> messageList;
    private Context context;
    private HalChat hc;
    private String password;
    private ChatActivity chatActivity;

    private static final int TYPE_SENT=1;
    private static final int TYPE_RECEIVED=2;
    private static final int TYPE_START_CHAT = 3;
    private static final int TYPE_JOIN=4;
    private static final int TYPE_EXIT=5;
    private static final String TAG="HCMA";
    private static final String PAYLOAD_REACTIONS="reactions";
    private static final String PAYLOAD_POLL="poll";

    //Sort system
    private final Set<Long> seenIds = new HashSet<>();
    private final Set<Long> pendingReactionLoads=ConcurrentHashMap.newKeySet();

    private static final Comparator<HCMessage> BY_UID =
            Comparator.comparingLong(HCMessage::getMsgUID);

    private final HCMessage startMessage;

    public MessageAdapter(List<HCMessage> messageList,ChatActivity chatActivity,HalChat hc,String password) {
        this.messageList = MessageHistory.sortedUnique(messageList);
        this.context=chatActivity;
        this.chatActivity=chatActivity;
        this.hc=hc;
        this.password=password;
        this.startMessage=new HCMessage(0,0,0,0,chatActivity.chat.created,0,0,"",null,null,null,null,null,false,false,true,-1,
                true,false,null,0,0,false,-1);
        this.messageList.add(startMessage);
        this.messageList.sort(BY_UID);
        rebuildSeenIds();
        setHasStableIds(true);
    }

    public void addMessageLoaded(HCMessage msg) {
        long uid=msg.getMsgUID();

        if(!seenIds.add(uid)) {
            return;
        }

        int pos = Collections.binarySearch(messageList, msg, BY_UID);
        if (pos < 0) pos = -pos - 1;

        messageList.add(pos, msg);
        notifyItemInserted(pos);

        // обновляем границы
    }

    public void addMessagesBatch(List<HCMessage> messages) {
        for(HCMessage message : MessageHistory.sortedUnique(messages)) {
            addMessageLoaded(message);
        }
    }

    HCMessage getOldestMessage() {
        return MessageHistory.oldestRealMessage(messageList);
    }

    HCMessage getNewestMessage() {
        return MessageHistory.newestRealMessage(messageList);
    }

    @Override
    public int getItemViewType(int position) {
        /*if (messageList.get(position).decryptedMessage.equals("[DATE]")) {
            return TYPE_DATE;
        }*/
        HCMessage msg=messageList.get(position);
        if(msg.type==2) {
            return TYPE_JOIN;
        } else if(msg.type==3) {
            return TYPE_EXIT;
        } else if(msg.type==-1) {
            return TYPE_START_CHAT;
        }
        return msg.isFrom?TYPE_SENT:TYPE_RECEIVED;
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        if (viewType == TYPE_SENT) {
            View view = LayoutInflater.from(context).inflate(R.layout.message_item_sent, parent, false);
            return new SentMessageViewHolder(view, this);
        } else if (viewType == TYPE_RECEIVED) {
            View view = LayoutInflater.from(context).inflate(R.layout.message_item_received, parent, false);
            return new ReceivedMessageViewHolder(view, this);
        } else if (viewType == TYPE_START_CHAT) {
            View view = LayoutInflater.from(context).inflate(R.layout.start_chat_item, parent, false);
            return new StartChatViewHolder(view);
        } else if(viewType==TYPE_JOIN) {
            View view=LayoutInflater.from(context).inflate(R.layout.join_chat_item, parent, false);
            return new JoinChatViewHolder(view);
        } else if(viewType==TYPE_EXIT) {
            View view=LayoutInflater.from(context).inflate(R.layout.exit_chat_item, parent, false);
            return new ExitChatViewHolder(view);
        }
        throw new IllegalArgumentException("Unknown view type: " + viewType);
    }

    private int pxToDP(int px) {
        return (int)(px / context.getResources().getDisplayMetrics().density);
    }

    /**
     * Очищает список сообщений и заменяет на новые предзагруженные.
     *
     * @param messages список сообщений
     * @returns void
     */
    protected void clearMessages(List<HCMessage> messages) {
        ArrayList<HCMessage> replacement=MessageHistory.sortedUnique(messages);
        replacement.add(startMessage);
        replacement.sort(BY_UID);
        this.messageList=replacement;
        rebuildSeenIds();
        notifyDataSetChanged();
    }

    private void rebuildSeenIds() {
        seenIds.clear();
        for (HCMessage message : messageList) {
            seenIds.add(message.getMsgUID());
        }
    }

    @Override
    public long getItemId(int position) {
        return messageList.get(position).getMsgUID();
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        HCMessage message = messageList.get(position);

        /*if(holder instanceof DateViewHolder) {
            DateViewHolder dateHolder=(DateViewHolder) holder;
            dateHolder.dateText.setText(convertDate(message.time*1000));
            return;
        }*/

        if(holder instanceof JoinChatViewHolder) {
            JoinChatViewHolder msgv=(JoinChatViewHolder) holder;
            msgv.boundMessageId=message.msgId;
            msgv.joinChatMsg.setText("Новый участник");

            boolean showDate;
            long msgDay = message.time / 86400;
            if (position == 0) {
                showDate = true;
            } else {
                long prevDay = messageList.get(position - 1).time / 86400;
                showDate = prevDay != msgDay;
            }

            if (showDate) {
                msgv.dateText.setText(convertDate(message.time*1000));
                msgv.dateBlock.setVisibility(View.VISIBLE);
            } else {
                msgv.dateBlock.setVisibility(View.GONE);
            }

            hc.hnUsers.getUserByUserId(message.fromId).thenAccept(fromUser-> {
                chatActivity.runOnUiThread(() -> {
                    if(fromUser!=null && msgv.boundMessageId==message.msgId) {
                        msgv.joinChatMsg.setText("Новый участник ~" + fromUser.nickname);
                    }
                });
            });
            return;
        } else if(holder instanceof ExitChatViewHolder) {
            ExitChatViewHolder msgv=(ExitChatViewHolder) holder;
            msgv.boundMessageId=message.msgId;
            msgv.exitChatMsg.setText("Ушёл участник");

            boolean showDate;
            long msgDay = message.time / 86400;
            if (position == 0) {
                showDate = true;
            } else {
                long prevDay = messageList.get(position - 1).time / 86400;
                showDate = prevDay != msgDay;
            }

            if (showDate) {
                msgv.dateText.setText(convertDate(message.time*1000));
                msgv.dateBlock.setVisibility(View.VISIBLE);
            } else {
                msgv.dateBlock.setVisibility(View.GONE);
            }

            hc.hnUsers.getUserByUserId(message.fromId).thenAccept(fromUser-> {
                chatActivity.runOnUiThread(() -> {
                    if(fromUser!=null && msgv.boundMessageId==message.msgId) {
                        msgv.exitChatMsg.setText("Ушёл участник ~" + fromUser.nickname);
                    }
                });
            });
            return;
        } else if(holder instanceof StartChatViewHolder) {
            StartChatViewHolder msgv=(StartChatViewHolder) holder;
            msgv.dateText.setText(convertDate(message.time*1000));
            msgv.dateBlock.setVisibility(View.VISIBLE);
            msgv.createdChat.setVisibility(View.VISIBLE);

            return;
        }

        MessageViewHolder msgv=(MessageViewHolder) holder;
        msgv.boundMessageId=message.msgId;
        long bindingVersion=++msgv.bindingVersion;
        msgv.avatar.setImageResource(R.drawable.ic_add_people);

        msgv.pollVariantsDiv.setVisibility(View.GONE);

        msgv.pixelDiv.setVisibility(View.GONE);

        msgv.messageText.setVisibility(View.VISIBLE);

        boolean showDate;
        long msgDay = message.time / 86400;
        if (position == 0) {
            showDate = true;
        } else {
            long prevDay = messageList.get(position - 1).time / 86400;
            showDate = prevDay != msgDay;
        }

        if (showDate) {
            msgv.dateText.setText(convertDate(message.time*1000));
            msgv.dateBlock.setVisibility(View.VISIBLE);
        } else {
            msgv.dateBlock.setVisibility(View.GONE);
        }

        //Polls
        bindPoll(msgv,message);

        hc.hnUsers.getUserByUserId(message.fromId).handle((fromUser,error)->{
            if(error!=null) {
                Log.e(TAG, "Unable to bind message author", error);
            }
            return fromUser;
        }).thenAccept(fromUser->
                chatActivity.runOnUiThread(() -> {
            if (msgv.boundMessageId != message.msgId || msgv.bindingVersion != bindingVersion) {
                return;
            }

            String authorName=chatActivity.chat.chatType==2
                    ? chatActivity.chat.name
                    : fromUser==null ? String.valueOf(message.fromId) : fromUser.nickname;
            msgv.nickname.setText(authorName);
            msgv.messageText.setText(HalChatFunctionsLib.replaceEmojis(context,msgv.messageText,hc,message.decryptedMessage),TextView.BufferType.SPANNABLE);
            msgv.messageTime.setText(HalChatGroupChatsMessages.convertTime(message.time));
            msgv.attachments.removeAllViews();
            msgv.answerMsgDiv.removeAllViews();
            msgv.commentsDiv.removeAllViews();
            msgv.recordMicDiv.removeAllViews();
            msgv.recordMicDiv.setVisibility(View.GONE);
            bindReactions(msgv,message,bindingVersion);
            if(!message.hasReactionData
                    && message.msgId>=0
                    && pendingReactionLoads.add(message.msgId)) {
                hc.chatGroupChats.updateMessage(message.chatId,message.msgId)
                        .whenComplete((updated,error)->
                                pendingReactionLoads.remove(message.msgId)
                        );
            }
        /*ViewGroup.LayoutParams params = msgv.messageLL.getLayoutParams();
        params.width = ViewGroup.LayoutParams.WRAP_CONTENT;
        msgv.messageLL.setLayoutParams(params);*/

            String avatarId=chatActivity.chat.chatType==2
                    ? chatActivity.chat.icon
                    : fromUser==null ? null : fromUser.icon;
            if(avatarId!=null) {
                hc.hd.getFileById(avatarId).thenAccept(fileIcon->{
                new Handler(Looper.getMainLooper()).post(() -> {
                    if(msgv.boundMessageId==message.msgId && msgv.bindingVersion==bindingVersion) {
                        Glide.with(context)
                                .load(fileIcon)
                                .override(150, 150)
                                .placeholder(R.drawable.ic_add_people)
                                .diskCacheStrategy(DiskCacheStrategy.ALL)
                                .skipMemoryCache(false)
                                .into(msgv.avatar);
                    }
                });
                }).exceptionally(error -> {
                    Log.e(TAG,"Unable to load message avatar "+avatarId,error);
                    return null;
                });
            }



            msgv.messageLL.setOnLongClickListener(v -> {
                chatActivity.showMessageContextMenu(message,v);
                return true;
            });


            if(message.answerMsg!=-1) {
                View answerMsg=LayoutInflater.from(context).inflate(R.layout.answer_msg,msgv.answerMsgDiv,false);
                TextView answerNickname=answerMsg.findViewById(R.id.answerNickname);
                TextView answerText=answerMsg.findViewById(R.id.answerText);

                final long answerMsgId=message.answerMsg;
                final HCMessage[] answerHCM = {hc.chatGroupChats.getMessageById(message.chatId, message.answerMsg)};
                if(answerHCM[0] ==null) {
                    //RETRY
                } else {
                    hc.hnUsers.getUserByUserId(answerHCM[0].fromId).thenAccept(answerUser->{
                        if(answerUser==null) {
                            return;
                        }
                        HCMessage decryptedAnswer=hc.chatGroupChats.decryptMessage(answerHCM[0],password);
                        chatActivity.runOnUiThread(() -> {
                            if(msgv.boundMessageId!=message.msgId || msgv.bindingVersion!=bindingVersion) {
                                return;
                            }
                            answerNickname.setText(answerUser.nickname);
                            answerText.setText(
                                    HalChatFunctionsLib.replaceEmojis(
                                            context,
                                            answerText,
                                            hc,
                                            decryptedAnswer.decryptedMessage
                                    ),
                                    TextView.BufferType.SPANNABLE
                            );
                            answerMsg.setOnClickListener(v -> {
                                //Move to msg
                                chatActivity.goToMessage(answerMsgId);
                            });
                            msgv.answerMsgDiv.addView(answerMsg);
                        });
                    });
                }
            }

            if(!message.recordMic.equals("-1")) {
                RecordedAudioView recordMicDiv = (RecordedAudioView) LayoutInflater.from(context).inflate(R.layout.recorded_audio_item,msgv.recordMicDiv,false);
                WaveformView waveformView = recordMicDiv.findViewById(R.id.waves);

                TextView textViewTimespanAudio=recordMicDiv.findViewById(R.id.textViewTimespanAudio);
                ImageButton imageButton=recordMicDiv.findViewById(R.id.playPauseBtn);

                hc.hd.getFileById(message.recordMic).thenAccept(file->{
                    String filePathAudio=file.getAbsolutePath();
                    chatActivity.runOnUiThread(() -> {
                        if(msgv.boundMessageId!=message.msgId || msgv.bindingVersion!=bindingVersion) {
                            return;
                        }
                        recordMicDiv.init(textViewTimespanAudio,waveformView,imageButton);
                        msgv.recordMicDiv.addView(recordMicDiv);
                        msgv.recordMicDiv.setVisibility(View.VISIBLE);

                        Rect visibleRect = new Rect();
                        waveformView.getLocalVisibleRect(visibleRect);
                        int visibleWidth = visibleRect.width();
                        int samplesCount=(visibleWidth - WaveformView.SPACE_BETWEEN) / (WaveformView.RECT_WIDTH + WaveformView.SPACE_BETWEEN);

                        TaskExecutorManager.getInstance().submitAudio("recordMic:"+message.recordMic,()->{
                            recordMicDiv.setAudio(chatActivity,filePathAudio);
                            int[] waveform=waveformView.generateWaveformFromAudioFile(filePathAudio, samplesCount);
                            chatActivity.runOnUiThread(() -> {
                                if(msgv.boundMessageId==message.msgId && msgv.bindingVersion==bindingVersion) {
                                    waveformView.setWaveformData(waveform);
                                }
                            });
                            return null;
                        });
                    });
                }).exceptionally(error -> {
                    Log.e(TAG,"Unable to load recorded message "+message.recordMic,error);
                    return null;
                });
            }

            if(message.attachments.length()>0) {
                for (int i = 0; i < message.attachments.length(); i++) {
                    try {
                        String attachmentId = message.attachments.getString(i);
                        hc.hd.getFileById(attachmentId).thenAccept(file -> {
                            HDFile hdFile = hc.hd.getHDFileById(attachmentId);
                            chatActivity.runOnUiThread(() -> {
                                if (hdFile != null
                                        && msgv.boundMessageId == message.msgId
                                        && msgv.bindingVersion == bindingVersion) {
                                    bindAttachment(msgv, file, hdFile);
                                }
                            });
                        }).exceptionally(error -> {
                            Log.e(TAG, "Unable to load attachment " + attachmentId, error);
                            return null;
                        });
                    } catch (JSONException e) {
                        Log.e(TAG, "MessageAdapter", e);
                    }
                }
            }

            if(message.type==1) {
                //View commentsDiv = LayoutInflater.from(context).inflate(R.layout.message_comments_count, msgv.messageLL, false);
                TextView commentsText = new TextView(context);
                commentsText.setText(context.getString(R.string.count_comments,hc.chatGroupChats.loadCountComments(message.chatId, message.msgId)));
                msgv.commentsDiv.addView(commentsText);
            }

            if(message.pixelId>0) {
                msgv.messageText.setVisibility(View.GONE);

                hc.hd.getFileById(hc.EPSystem.getPixelById(message.pixelId).image).thenAccept(file->{
                    new Handler(Looper.getMainLooper()).post(() -> {
                        if(msgv.boundMessageId==message.msgId && msgv.bindingVersion==bindingVersion) {
                            msgv.pixelDiv.setVisibility(View.VISIBLE);
                            Glide.with(context)
                                    .asDrawable()
                                    .load(file)
                                    .diskCacheStrategy(DiskCacheStrategy.ALL)
                                    .skipMemoryCache(false)
                                    .placeholder(R.drawable.image)
                                    .error(R.drawable.image)
                                    .into(msgv.pixelDiv);
                        }
                    });

                });
            }
        }));

    }

    private void bindAttachment(MessageViewHolder holder, File file, HDFile hdFile) {
        View itemFile;
        ImageView fileIcon;
        String fileType = hdFile.fileType;

        if ("image".equals(fileType)) {
            itemFile = LayoutInflater.from(context).inflate(
                    R.layout.file_image_item,
                    holder.attachments,
                    false
            );
            fileIcon = itemFile.findViewById(R.id.fileicon);

            if (hdFile.imageData != null) {
                int originalWidth = hdFile.imageData.optInt("width");
                int originalHeight = hdFile.imageData.optInt("height");

                if (originalWidth > 0 && originalHeight > 0) {
                    int maxWidthPx = (int) (250 * context.getResources().getDisplayMetrics().density);
                    float aspectRatio = (float) originalHeight / originalWidth;
                    int targetWidth = Math.min(originalWidth, maxWidthPx);
                    int targetHeight = (int) (targetWidth * aspectRatio);

                    ViewGroup.LayoutParams params = fileIcon.getLayoutParams();
                    if (params == null) {
                        params = new ViewGroup.LayoutParams(targetWidth, targetHeight);
                    } else {
                        params.width = targetWidth;
                        params.height = targetHeight;
                    }
                    fileIcon.setLayoutParams(params);
                }
            }

            Glide.with(context)
                    .asDrawable()
                    .load(file)
                    .diskCacheStrategy(DiskCacheStrategy.ALL)
                    .skipMemoryCache(false)
                    .placeholder(R.drawable.image)
                    .error(R.drawable.image)
                    .into(fileIcon);
        } else {
            itemFile = LayoutInflater.from(context).inflate(
                    R.layout.file_item,
                    holder.attachments,
                    false
            );
            fileIcon = itemFile.findViewById(R.id.fileicon);
            fileIcon.setImageDrawable(
                    AppCompatResources.getDrawable(context, hc.hd.getFileIcon(fileType))
            );
            TextView fileName = itemFile.findViewById(R.id.filename);
            fileName.setText(hdFile.name);
        }

        itemFile.setOnClickListener(v -> openFile(hdFile.id, hdFile.name));
        holder.attachments.addView(itemFile);
    }

    private void bindPoll(MessageViewHolder holder,HCMessage message) {
        holder.pollVariantsDiv.setVisibility(View.GONE);
        if(message.type!=4)return;

        List<String> variants=message.getPollVariants();
        if(variants==null)return;

        if(message.msgId>=0&&!message.hasPollResult) {
            message.selectedPollVariant=hc.chatGroupChats.getSelectedPollVariant(message.chatId,message.msgId);
            message.hasPollResult=true;
        }
        for(int i=0;i<12;i++) {
            View pollVariant=holder.pollVariantsDiv.getChildAt(i);
            if(pollVariant!=null) {
                pollVariant.setVisibility(View.GONE);
                pollVariant.setBackgroundResource(R.drawable.poll_variant);
            }
        }

        int count=Math.min(variants.size(),12);
        for(int i=0;i<count;i++) {
            LinearLayout pollVarLL=(LinearLayout)holder.pollVariantsDiv.getChildAt(i);
            TextView pollText=(TextView)pollVarLL.getChildAt(0);
            TextView pollCount=(TextView)pollVarLL.getChildAt(1);

            pollText.setText(variants.get(i));
            pollCount.setText(String.valueOf(hc.chatGroupChats.getVotesPollVariant(message.chatId,message.msgId,i)));
            pollVarLL.setBackgroundResource(
                    message.selectedPollVariant==i
                            ? R.drawable.poll_variant_selected
                            : R.drawable.poll_variant
            );
            pollVarLL.setVisibility(View.VISIBLE);
        }
        holder.pollVariantsDiv.setVisibility(View.VISIBLE);
    }

    private void bindReactions(MessageViewHolder holder,HCMessage message,long bindingVersion) {
        holder.messageReactionsDiv.removeAllViews();
        holder.messageReactionsDiv.setVisibility(View.GONE);
        if(message.reactions==null || message.reactions.isEmpty()) {
            return;
        }

        for(MessageReaction reaction : message.reactions) {
            View reactionView=LayoutInflater.from(context).inflate(
                    R.layout.message_reaction_item,
                    holder.messageReactionsDiv,
                    false
            );
            ImageView reactionEmoji=reactionView.findViewById(R.id.reaction_emoji);
            TextView reactionCount=reactionView.findViewById(R.id.reaction_count);
            reactionCount.setText(String.valueOf(reaction.count));
            reactionEmoji.setImageResource(R.drawable.ic_robot);

            if(message.selectedReaction==reaction.emojiId) {
                reactionView.setBackgroundResource(R.drawable.message_reaction_background_selected);
            } else {
                reactionView.setBackgroundResource(R.drawable.message_reaction_background);
            }

            reactionView.setOnClickListener(view->
                    chatActivity.setMessageReaction(message,reaction.emojiId)
            );
            holder.messageReactionsDiv.addView(reactionView);

            hc.EPSystem.getEmojiByIdAsync(reaction.emojiId).thenAccept(emoji->{
                if(emoji==null) {
                    return;
                }
                hc.hd.getFileById(emoji.image).thenAccept(file->
                    chatActivity.runOnUiThread(()->{
                        if(holder.boundMessageId==message.msgId
                                && holder.bindingVersion==bindingVersion) {
                            Glide.with(context)
                                    .load(file)
                                    .diskCacheStrategy(DiskCacheStrategy.ALL)
                                    .skipMemoryCache(false)
                                    .placeholder(R.drawable.ic_robot)
                                    .into(reactionEmoji);
                        }
                    })
                );
            });
        }
        holder.messageReactionsDiv.setVisibility(View.VISIBLE);
    }

    private void openFile(String id,String name) {
        hc.hd.getFileById(id).thenAccept(file->{
            String mimeType = getMimeType(name);
            if (mimeType == null) {
                mimeType = "*/*";
            }
            Uri fileUri;
            fileUri = FileProvider.getUriForFile(context,
                    context.getPackageName() + ".provider", file);

            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(fileUri, mimeType);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

            context.startActivity(Intent.createChooser(intent, "Открыть файл с помощью"));
        });
    }

    private static String getMimeType(String name) {
        int lastDotIndex = name.lastIndexOf('.');
        if (lastDotIndex != -1 && lastDotIndex < name.length() - 1) {
            String extension = name.substring(lastDotIndex + 1).toLowerCase();
            return MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
        }

        return null;
    }

    @Override
    public int getItemCount() {
        return messageList.size();
    }

    private String convertDate(long timestamp) {
        java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("dd.MM.yyyy", Locale.getDefault());
        return sdf.format(new java.util.Date(timestamp));
    }

    static class MessageViewHolder extends RecyclerView.ViewHolder {
        long boundMessageId = Long.MIN_VALUE;
        long bindingVersion = 0;
        ImageView avatar;
        TextView messageText, messageTime,nickname;
        LinearLayout answerMsgDiv;
        FlexboxLayout attachments;
        LinearLayout commentsDiv;
        LinearLayout recordMicDiv;
        LinearLayout messageLL;
        LinearLayout dateBlock;
        LinearLayout pollVariantsDiv;
        FlexboxLayout messageReactionsDiv;
        TextView dateText;
        ImageView pixelDiv;

        public MessageViewHolder(@NonNull View itemView, MessageAdapter adapter) {
            super(itemView);
            messageText = itemView.findViewById(R.id.message_text);
            messageTime = itemView.findViewById(R.id.message_time);
            answerMsgDiv=itemView.findViewById(R.id.answerMsgDiv);
            attachments=itemView.findViewById(R.id.attachments);
            nickname=itemView.findViewById(R.id.nicknameMsg);
            avatar=itemView.findViewById(R.id.icon);
            commentsDiv=itemView.findViewById(R.id.countCommentsDiv);
            recordMicDiv=itemView.findViewById(R.id.recordMicDiv);
            messageLL=itemView.findViewById(R.id.messageLL);
            dateBlock=itemView.findViewById(R.id.dateBlock);
            dateText=itemView.findViewById(R.id.dateText);
            pixelDiv=itemView.findViewById(R.id.pixelDiv);
            pollVariantsDiv=itemView.findViewById(R.id.pollVariantsDiv);
            messageReactionsDiv=itemView.findViewById(R.id.messageReactionsDiv);

            commentsDiv.setOnClickListener(v -> {
                //Open comments
                int pos = getAdapterPosition();
                if (pos != RecyclerView.NO_POSITION) {
                    HCMessage msg = adapter.messageList.get(pos);
                    adapter.chatActivity.openComments(msg.msgId);
                }
            });

            for (int i=0;i<12;i++) {
                View pollVarLL = pollVariantsDiv.getChildAt(i);
                if (pollVarLL != null) {
                    pollVarLL.setTag(i);
                    pollVarLL.setOnClickListener(v -> {
                        int pos = getAdapterPosition();
                        if (pos != RecyclerView.NO_POSITION) {
                            HCMessage message = adapter.messageList.get(pos);
                            int variantIndex=(int)v.getTag();
                            long oldVariant=message.selectedPollVariant;
                            message.selectedPollVariant=oldVariant==variantIndex?-1:variantIndex;
                            message.hasPollResult=true;
                            adapter.notifyItemChanged(pos,PAYLOAD_POLL);
                            adapter.hc.chatGroupChats.votePollVariant(message.chatId,message.msgId,variantIndex).whenComplete((b,error)->{
                                adapter.chatActivity.runOnUiThread(()->{
                                    int currentPosition=adapter.getPositionById(message.msgId);
                                    if(currentPosition==-1)return;
                                    HCMessage currentMessage=adapter.messageList.get(currentPosition);
                                    currentMessage.selectedPollVariant=error==null&&Boolean.TRUE.equals(b)
                                            ? adapter.hc.chatGroupChats.getSelectedPollVariant(message.chatId,message.msgId)
                                            : oldVariant;
                                    currentMessage.hasPollResult=true;
                                    adapter.notifyItemChanged(currentPosition,PAYLOAD_POLL);
                                });
                            });
                        }
                    });
                }
            }
        }
    }

    @Override
    public void onBindViewHolder(
            @NonNull RecyclerView.ViewHolder holder,
            int position,
            @NonNull List<Object> payloads
    ) {
        if(payloads.contains(PAYLOAD_REACTIONS) && holder instanceof MessageViewHolder) {
            HCMessage message=messageList.get(position);
            MessageViewHolder messageHolder=(MessageViewHolder)holder;
            messageHolder.boundMessageId=message.msgId;
            bindReactions(messageHolder,message,messageHolder.bindingVersion);
            return;
        }
        if(payloads.contains(PAYLOAD_POLL) && holder instanceof MessageViewHolder) {
            HCMessage message=messageList.get(position);
            MessageViewHolder messageHolder=(MessageViewHolder)holder;
            messageHolder.boundMessageId=message.msgId;
            bindPoll(messageHolder,message);
            return;
        }
        super.onBindViewHolder(holder,position,payloads);
    }


    static class SentMessageViewHolder extends MessageViewHolder {

        public SentMessageViewHolder(@NonNull View itemView, MessageAdapter adapter) {
            super(itemView, adapter);
        }
    }

    static class ReceivedMessageViewHolder extends MessageViewHolder {
        public ReceivedMessageViewHolder(@NonNull View itemView, MessageAdapter adapter) {
            super(itemView, adapter);
        }
    }

    static class JoinChatViewHolder extends RecyclerView.ViewHolder {
        long boundMessageId = Long.MIN_VALUE;
        TextView joinChatMsg;
        LinearLayout messageLL;
        LinearLayout dateBlock;
        TextView dateText;

        public JoinChatViewHolder(@NonNull View itemView) {
            super(itemView);
            joinChatMsg=itemView.findViewById(R.id.joinChatMsg);
            messageLL=itemView.findViewById(R.id.messageLL);
            dateBlock=itemView.findViewById(R.id.dateBlock);
            dateText=itemView.findViewById(R.id.dateText);
        }
    }

    static class ExitChatViewHolder extends RecyclerView.ViewHolder {
        long boundMessageId = Long.MIN_VALUE;
        TextView exitChatMsg;
        LinearLayout messageLL;
        LinearLayout dateBlock;
        TextView dateText;

        public ExitChatViewHolder(@NonNull View itemView) {
            super(itemView);
            exitChatMsg=itemView.findViewById(R.id.exitChatMsg);
            messageLL=itemView.findViewById(R.id.messageLL);
            dateBlock=itemView.findViewById(R.id.dateBlock);
            dateText=itemView.findViewById(R.id.dateText);
        }
    }

    static class StartChatViewHolder extends RecyclerView.ViewHolder {
        LinearLayout dateBlock;
        TextView dateText;
        LinearLayout createdChat;

        public StartChatViewHolder(@NonNull View itemView) {
            super(itemView);
            dateBlock=itemView.findViewById(R.id.dateBlock);
            dateText=itemView.findViewById(R.id.dateText);
            createdChat=itemView.findViewById(R.id.createdChat);
        }
    }

    //ACTION MESSAGE
    protected void deleteMessage(long msgId) {
        for(int i=0;i<messageList.size();i++) {
            long mid=messageList.get(i).msgId;
            if(msgId==mid) {
                HCMessage removed=messageList.remove(i);
                seenIds.remove(removed.getMsgUID());
                notifyItemRemoved(i);
                return;
            }
        }
    }

    protected void editMessage(HCMessage newmsg) {
        long msgId=newmsg.msgId;
        for(int i=0;i<messageList.size();i++) {
            long mid=messageList.get(i).msgId;
            if(msgId==mid) {
                messageList.set(i,newmsg);
                notifyItemChanged(i);
                return;
            }
        }
    }

    protected HCMessage getMessageById(long msgId) {
        for(HCMessage message : messageList) {
            if(message.msgId==msgId)return message;
        }
        return null;
    }

    protected void updateMessageReactions(long msgId) {
        int position=getPositionById(msgId);
        if(position!=-1)notifyItemChanged(position,PAYLOAD_REACTIONS);
    }

    protected void updatePollResults() {
        for(int i=0;i<messageList.size();i++) {
            HCMessage message=messageList.get(i);
            if(message.type!=4)continue;
            message.selectedPollVariant=hc.chatGroupChats.getSelectedPollVariant(message.chatId,message.msgId);
            message.hasPollResult=true;
            notifyItemChanged(i,PAYLOAD_POLL);
        }
    }

    public int getPositionById(long msgId) {
        for (int i = 0; i < messageList.size(); i++) {
            if (messageList.get(i).msgId==msgId) {
                return i;
            }
        }
        return -1;
    }
}
