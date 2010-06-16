package com.ssb.droidsound;

import android.app.Activity;
import android.content.Context;
import android.database.Cursor;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CursorAdapter;
import android.widget.TextView;

public class SongListAdapter extends BaseAdapter {
	private static final String TAG = CursorAdapter.class.getSimpleName();

	private Cursor mCursor;
	private int headerIndex;
	private int itemIndex;

	private Context mContext;

	public SongListAdapter(Context context, Cursor c) {
		super();
		mCursor = c;
		mContext = context;
		headerIndex = mCursor.getColumnIndex("COMPOSER");
		itemIndex = mCursor.getColumnIndex("TITLE");
	}

	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return mCursor.getCount();
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		LayoutInflater inflater = ((Activity)mContext).getLayoutInflater();
		String oldHeader = null;
		if(position > 0) {
			mCursor.moveToPosition(position-1);
			oldHeader = mCursor.getString(headerIndex);
			mCursor.moveToNext();
		} else {
			mCursor.moveToPosition(position);
		}
		
		String item = mCursor.getString(itemIndex);
		String header = mCursor.getString(headerIndex);
		
		if(convertView == null) {
			convertView = inflater.inflate(R.layout.songlist_header, null);
		}
		TextView headerView = (TextView)((ViewGroup)convertView).getChildAt(0);
		TextView itemView = (TextView)((ViewGroup)convertView).getChildAt(1);
		if(!header.equals(oldHeader)) {
			headerView.setText(header);
			headerView.setVisibility(View.VISIBLE);
		} else {
			headerView.setVisibility(View.GONE);
		}
		itemView.setText(item);
		
		return convertView;
		
	}
}

/*
	@Override
	public void bindView(View view, Context context, Cursor cursor) {

		boolean withHeader = false;
		int pos = cursor.getPosition();
		
		Log.v(TAG, String.format("bindView on pos %d", pos));
		
		String item = cursor.getString(itemIndex);
		String header = cursor.getString(headerIndex);
		
		if(cursor.moveToPrevious()) {
			withHeader = header.equals(cursor.getString(headerIndex));
		} else {
			withHeader = true;
		}
		
		Log.v(TAG, String.format("Now at pos %d Headers %s", cursor.getPosition(), Boolean.toString(withHeader)));

		if(withHeader) {
			TextView headerView = (TextView)((ViewGroup)view).getChildAt(0);
			TextView itemView = (TextView)((ViewGroup)view).getChildAt(1);
			headerView.setText(header);
			itemView.setText(item);
		} else {
			((TextView)view).setText(item);
		}
		cursor.moveToPosition(pos);
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent) {
		View view;
		boolean withHeader = false;
		int pos = cursor.getPosition();
		
		Log.v(TAG, String.format("newView on pos %d", pos));

		String item = cursor.getString(itemIndex);
		String header = cursor.getString(headerIndex);
		LayoutInflater inflater = ((Activity)context).getLayoutInflater();
		
		
		if(cursor.moveToPrevious()) {
			withHeader = header.equals(cursor.getString(headerIndex));
			Log.v(TAG, String.format("After prev at pos %d", cursor.getPosition()));
		} else {
			withHeader = true;
		}
		
		Log.v(TAG, String.format("Now at pos %d Headers %s", cursor.getPosition(), Boolean.toString(withHeader)));

		
		if(withHeader) {
			view = inflater.inflate(R.layout.songlist_header, null);
			TextView headerView = (TextView)((ViewGroup)view).getChildAt(0);
			TextView itemView = (TextView)((ViewGroup)view).getChildAt(1);
			headerView.setText(header);
			itemView.setText(item);
		} else {		
			view = inflater.inflate(R.layout.songlist_item, null);
			((TextView)view).setText(item);
		}
		
		cursor.moveToPosition(pos);
		
		return view;
	}

	@Override
	public int getViewTypeCount() {
		return 2;
	}
	
	@Override
	public int getItemViewType(int position) {
		int rc = 1;
		int oldpos = mCursor.getPosition();

		if(position > 0) {
			if(!mCursor.moveToPosition(position-1)) {
				Log.v(TAG, String.format("Could not MOVE"));
			}
			String s = mCursor.getString(headerIndex);
			Log.v(TAG, String.format("Prev header is %s", s));
			if(mCursor.moveToNext()) {
				String s1 = mCursor.getString(headerIndex);
				Log.v(TAG, String.format("This header is %s", s1));
				rc = s.equals(s1) ? 0 : 1;
			} else {
				rc = 0;
			}
		}
		Log.v(TAG, String.format("getItemViewType on pos %d => %d", position, rc));
		mCursor.moveToPosition(oldpos);
		return rc;
		
		
		
	} 
} */
