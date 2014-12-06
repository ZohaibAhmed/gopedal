class ApiController < ApplicationController
	protect_from_forgery :except => ["fetch_location", "location", "send_to_arduino", "toggleLock"]

	# GET fetch_location
	# initial call to fetch from arduino
	def fetch_location
		client = initTwilioClient()
		# we need to fetch the location from the device...
		# send an SMS to the device asking for location
		client.account.messages.create(
	      :from => "+0000000000",
	      :to => "+0000000000",
	      :body => "LOCATION"
	    )

		render :json => {"status" => "OK"}
	end

	# GET location
	# gets the latest location from the database
	def location
		render :json => {"latitude" => Location.last.latitude, "longitude" => Location.last.longitude}
	end

	def toggleLock
		client = initTwilioClient()
		# get the bike
		bike = Bike.first

		if bike.lock
			# this bike was locked, unlock it
			bike.lock = false
			bike.save

			# tell the arduino that it doesn't need to keep me updated about this bike.
			client.account.messages.create(
		      :from => "+0000000000",
		      :to => "+0000000000",
		      :body => "UNLOCK"
	    	)
		else
			bike.lock = true
			bike.save

			# tell the arduino that it needs to tell me about the bike location
			client.account.messages.create(
		      :from => "+0000000000",
		      :to => "+0000000000",
		      :body => "LOCK"
	    	)
		end

		if bike.lock
			st = "Your Bike is Locked"
		else
			st = "Your Bike is Unlocked"
		end

		render :json => {"status" => st}
	end

	# Send Phone Call to GoPedal
	def send_to_arduino
		# set up twilio
		client = initTwilioClient()

		event_type = params[:type]
		event_number = params[:number]
		event_message = params[:message]
		event_name = params[:name]

		# phone:
		# Incoming call from event_number

		# SMS:
		# event_name/event_number: event_message

		if event_type == "phone"
			client.account.messages.create(
		      :from => "+0000000000",
		      :to => "+0000000000",
		      :body => "Incoming call from\n #{event_number}"
		    )
		else
			# this is a sms
			if event_name.nil?
				client.account.messages.create(
			      :from => "+0000000000",
			      :to => "+0000000000",
			      :body => "#{event_number}:\n #{event_message}"
			    )
			else
				client.account.messages.create(
			      :from => "+0000000000",
			      :to => "+0000000000",
			      :body => "#{event_name}:\n #{event_message}"
			    )
			end
		end


		render :json => {"status" => "OK"}.to_json
	end
end
