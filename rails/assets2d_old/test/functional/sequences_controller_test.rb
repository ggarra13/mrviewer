require File.dirname(__FILE__) + '/../test_helper'
require 'sequences_controller'

# Re-raise errors caught by the controller.
class SequencesController; def rescue_action(e) raise e end; end

class SequencesControllerTest < Test::Unit::TestCase
  fixtures :sequences

  def setup
    @controller = SequencesController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = sequences(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:sequences)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:sequence)
    assert assigns(:sequence).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:sequence)
  end

  def test_create
    num_sequences = Sequence.count

    post :create, :sequence => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_sequences + 1, Sequence.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:sequence)
    assert assigns(:sequence).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Sequence.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Sequence.find(@first_id)
    }
  end
end
