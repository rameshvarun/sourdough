#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

#include <limits>

using namespace std;

double curr_window_size = 5; //Slow start, so begin with 5
unsigned int multiplicative_factor = 2; // The factor by which we decrease our window during a congestion event
unsigned int additive_factor = 1; // The factor by which we increase our window during a congestion event
unsigned int prop_delay_threshold = 155; // one-way propogation time threshold, for congestion event detection

unsigned int MIN_WINDOW_SIZE = 5;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = (unsigned int) curr_window_size;
  if (the_window_size < MIN_WINDOW_SIZE) the_window_size = MIN_WINDOW_SIZE;

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
  /* Multiplicative decrease, when congestion is detected. */
  curr_window_size /= multiplicative_factor;
  if (curr_window_size < 1) curr_window_size = 1;
  // if (debug_) {
    cout << "Congestion event detected." << endl;
  // }
}

double base_delay = std::numeric_limits<double>::infinity();
double TARGET_DELAY = 20;
double GAIN = .05;

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
  double delay = (double)recv_timestamp_acked - (double)send_timestamp_acked;
  base_delay = std::min(base_delay, delay);
  double queueing_delay = delay - base_delay;

  double off_target = TARGET_DELAY - queueing_delay;
  curr_window_size += GAIN * off_target / curr_window_size;
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
