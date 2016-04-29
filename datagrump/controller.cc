#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#include <fstream>
#include <limits>

using namespace std;

double curr_window_size = 5; //Slow start, so begin with 5
unsigned int multiplicative_factor = 2; // The factor by which we decrease our window during a congestion event
unsigned int additive_factor = 1; // The factor by which we increase our window during a congestion event
unsigned int prop_delay_threshold = 155; // one-way propogation time threshold, for congestion event detection

unsigned int MIN_WINDOW_SIZE = 5;

std::ofstream window_size_log;
std::ofstream queueing_delay_log;
std::ofstream queueing_delay_gradient_log;
std::ofstream queueing_delay_forecast_log;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{
  if (diagnostics_) {
    window_size_log.open("congestion_window_size.log", std::ofstream::out | std::ofstream::trunc);
    queueing_delay_log.open("queueing_delay.log", std::ofstream::out | std::ofstream::trunc);
    queueing_delay_gradient_log.open("queueing_delay_gradient.log", std::ofstream::out | std::ofstream::trunc);
    queueing_delay_forecast_log.open("queueing_delay_forecast.log", std::ofstream::out | std::ofstream::trunc);
  }
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) curr_window_size;
  if (the_window_size < MIN_WINDOW_SIZE) the_window_size = MIN_WINDOW_SIZE;

  if (diagnostics_) {
    window_size_log << the_window_size << endl;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms() << " window size is " <<
      the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{



  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

void Controller::on_timeout( void )
{
  if(_debug) {
    cout << "Timeout occurred." << endl;
  }
}

double base_delay = std::numeric_limits<double>::infinity();
double TARGET_DELAY = 20;

double INCREASE_GAIN = .10;
double DECREASE_GAIN = .10;

double prev_queueing_delay = 0;
double queueing_delay_gradient = 0;
double alpha = 0.9;

double PANIC_DELAY = 150;

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Calculate the queueing delay by subtracting the base delay (smallest
     measurement we have recieved so far) from the delay measurment. */
  double delay = (double)recv_timestamp_acked - (double)send_timestamp_acked;
  base_delay = std::min(base_delay, delay);
  double queueing_delay = delay - base_delay;

  /* Update our estimate of the gradient. */
  double new_delay_diff = queueing_delay - prev_queueing_delay;
  queueing_delay_gradient = (1 - alpha) * queueing_delay_gradient + alpha * new_delay_diff;
  prev_queueing_delay = queueing_delay;

  /* Predict the queueing_delay in the future. */
  double queueing_delay_forecast = std::max(queueing_delay + queueing_delay_gradient, 0.0);

  if (diagnostics_) {
    queueing_delay_log << queueing_delay << endl;
    queueing_delay_gradient_log << queueing_delay_gradient << endl;
    queueing_delay_forecast_log << queueing_delay_forecast << endl;
  }

  if (queueing_delay > PANIC_DELAY) {
    curr_window_size = 1;
    cout << "Panic." << endl;
  } else {
    double off_target = TARGET_DELAY - queueing_delay_forecast;

    if (off_target > 0) curr_window_size += INCREASE_GAIN * off_target / curr_window_size;
    else curr_window_size += DECREASE_GAIN * off_target / curr_window_size;
  }
  if (curr_window_size < 1) curr_window_size = 1;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
   << " received ack for datagram " << sequence_number_acked
   << " (send @ time " << send_timestamp_acked
   << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
   << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 200; /* timeout of one second */
}
