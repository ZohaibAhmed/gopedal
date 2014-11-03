package com.example.zohaibahmed.gopedal;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import at.abraxas.amarino.Amarino;
import at.abraxas.amarino.AmarinoIntent;
import com.google.android.gms.maps.*;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.MarkerOptions;
import android.widget.Toast;

public class MainActivity extends Activity {

    /** local */
    GoogleMap googleMap;
    private static final String DEVICE_ADDRESS =  "00:06:66:03:73:7B";

    private ArduinoReceiver arduinoReceiver = new ArduinoReceiver();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        createMapView();
        addMarker();

        PhoneCallListener phoneListener = new PhoneCallListener();
        TelephonyManager telephonyManager = (TelephonyManager) this
                .getSystemService(Context.TELEPHONY_SERVICE);
        telephonyManager.listen(phoneListener,
                PhoneStateListener.LISTEN_CALL_STATE);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        Toast.makeText(getApplicationContext(),
                "Tapped!",Toast.LENGTH_SHORT).show();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStart() {
        super.onStart();
        // in order to receive broadcasted intents we need to register our receiver
        registerReceiver(arduinoReceiver, new IntentFilter(AmarinoIntent.ACTION_RECEIVED));

        // this is how you tell Amarino to connect to a specific BT device from within your own code
        Amarino.connect(this, DEVICE_ADDRESS);
    }

    @Override
    protected void onStop() {
        super.onStop();

        // if you connect in onStart() you must not forget to disconnect when your app is closed
        Amarino.disconnect(this, DEVICE_ADDRESS);

        // do never forget to unregister a registered receiver
        unregisterReceiver(arduinoReceiver);
    }

    private void createMapView() {
        /**
         * Catch the null pointer exception that
         * may be thrown when initialising the map
         */
        try {
            if(null == googleMap){
                googleMap = ((MapFragment) getFragmentManager().findFragmentById(
                        R.id.mapView)).getMap();

                /**
                 * If the map is still null after attempted initialisation,
                 * show an error to the user
                 */
                if(null == googleMap) {
                    Toast.makeText(getApplicationContext(),
                            "Error creating map",Toast.LENGTH_SHORT).show();
                }
            }
        } catch (NullPointerException exception){
            Log.e("mapApp", exception.toString());
        }
    }

    /**
     * Adds a marker to the map
     */
    private void addMarker(){

        /** Make sure that the map has been initialised **/
        if(null != googleMap){
            googleMap.addMarker(new MarkerOptions()
                            .position(new LatLng(0, 0))
                            .title("Marker")
                            .draggable(true)
            );
        }
    }

    /**
     * ArduinoReceiver is responsible for catching broadcasted Amarino
     * events.
     *
     * It extracts data from the intent and updates the graph accordingly.
     */
    public class ArduinoReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String data = null;

            // the device address from which the data was sent, we don't need it here but to demonstrate how you retrieve it
            final String address = intent.getStringExtra(AmarinoIntent.EXTRA_DEVICE_ADDRESS);

            // the type of data which is added to the intent
            final int dataType = intent.getIntExtra(AmarinoIntent.EXTRA_DATA_TYPE, -1);

            // we only expect String data though, but it is better to check if really string was sent
            // later Amarino will support differnt data types, so far data comes always as string and
            // you have to parse the data to the type you have sent from Arduino, like it is shown below
            if (dataType == AmarinoIntent.STRING_EXTRA){
                data = intent.getStringExtra(AmarinoIntent.EXTRA_DATA);

                if (data != null){
                    Log.i("FROM ARDUINO: ", data);
                }
            }
        }
    }


}
