class ApplicationController < ActionController::Base
  # Prevent CSRF attacks by raising an exception.
  # For APIs, you may want to use :null_session instead.
  protect_from_forgery with: :exception

  def initTwilioClient
  	twilio_sid = 'twiliosid'
  	twilio_token = 'twiliotoken'

  	twilio_client = Twilio::REST::Client.new twilio_sid, twilio_token
  	return twilio_client
  end

end
