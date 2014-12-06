class RecieveController < ApplicationController
	protect_from_forgery :except => ["process_sms"]

	def process_sms
		
		@city = params[:FromCity].capitalize
    	@state = params[:FromState]
    	message_body = params["Body"]
    	from_number = params["From"]

    	# TODO: handle incoming
        # Handle Location
    	if message_body.split()[0] == "LOCATION"
    		# update location
    		if message_body.split()[1].to_f > 0.0 and message_body.split()[2].to_f > 0.0
    			Location.create(:latitude => message_body.split()[1], :longitude => message_body.split()[2])
    		end
    	end

    	render :json => {"from" => @from_number, "message_body" => message_body}.to_json
	end
end
