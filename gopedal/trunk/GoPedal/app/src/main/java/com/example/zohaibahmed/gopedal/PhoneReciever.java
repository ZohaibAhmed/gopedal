package com.example.zohaibahmed.gopedal;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

/**
 * Created by zohaibahmed on 14-11-03.
 */
public class PhoneReciever extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if(intent.getAction().equals("android.intent.action.PHONE_STATE")) {
            Bundle bundle = intent.getExtras();
            Log.i("RING RING", "RINGING in here");
        }
    }
}
